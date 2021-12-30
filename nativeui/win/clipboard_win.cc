// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/clipboard.h"

#include <algorithm>

#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/win/gdiplus.h"
#include "nativeui/win/drag_drop/clipboard_util.h"
#include "nativeui/win/util/win32_window.h"

namespace nu {

namespace {

// A scoper to impersonate the anonymous token and revert when leaving scope.
class AnonymousImpersonator {
 public:
  AnonymousImpersonator() {
    must_revert_ = ::ImpersonateAnonymousToken(::GetCurrentThread());
  }

  ~AnonymousImpersonator() {
    if (must_revert_)
      ::RevertToSelf();
  }

  AnonymousImpersonator& operator=(const AnonymousImpersonator&) = delete;
  AnonymousImpersonator(const AnonymousImpersonator&) = delete;

 private:
  BOOL must_revert_;
};

// A scoper to manage acquiring and automatically releasing the clipboard.
class ScopedClipboard {
 public:
  ScopedClipboard() : opened_(false) { }

  ~ScopedClipboard() {
    if (opened_)
      Release();
  }

  bool Acquire(HWND owner) {
    const int kMaxAttemptsToOpenClipboard = 5;

    if (opened_) {
      NOTREACHED();
      return false;
    }

    // Attempt to open the clipboard, which will acquire the Windows clipboard
    // lock.  This may fail if another process currently holds this lock.
    // We're willing to try a few times in the hopes of acquiring it.
    //
    // This turns out to be an issue when using remote desktop because the
    // rdpclip.exe process likes to read what we've written to the clipboard and
    // send it to the RDP client.  If we open and close the clipboard in quick
    // succession, we might be trying to open it while rdpclip.exe has it open,
    // See Bug 815425.
    //
    // In fact, we believe we'll only spin this loop over remote desktop.  In
    // normal situations, the user is initiating clipboard operations and there
    // shouldn't be contention.

    for (int attempts = 0; attempts < kMaxAttemptsToOpenClipboard; ++attempts) {
      // If we didn't manage to open the clipboard, sleep a bit and be hopeful.
      if (attempts != 0)
        ::Sleep(5);

      if (::OpenClipboard(owner)) {
        opened_ = true;
        return true;
      }
    }

    // We failed to acquire the clipboard.
    return false;
  }

  void Release() {
    if (opened_) {
      // Impersonate the anonymous token during the call to CloseClipboard
      // This prevents Windows 8+ capturing the broker's access token which
      // could be accessed by lower-privileges chrome processes leading to
      // a risk of EoP
      AnonymousImpersonator impersonator;
      ::CloseClipboard();
      opened_ = false;
    } else {
      NOTREACHED();
    }
  }

 private:
  bool opened_;
};

class ClipboardWindow : public Win32Window {
 public:
  explicit ClipboardWindow(Clipboard* delegate)
      : Win32Window(L"", HWND_MESSAGE, 0),
        delegate_(delegate) {}

 private:
  bool ProcessWindowMessage(HWND window,
                            UINT message,
                            WPARAM w_param,
                            LPARAM l_param,
                            LRESULT* result) override {
    switch (message) {
    case WM_RENDERFORMAT:
      // This message comes when SetClipboardData was sent a null data handle
      // and now it's come time to put the data on the clipboard.
      // We always set data, so there isn't a need to actually do anything here.
      break;
    case WM_RENDERALLFORMATS:
      // This message comes when SetClipboardData was sent a null data handle
      // and now this application is about to quit, so it must put data on
      // the clipboard before it exits.
      // We always set data, so there isn't a need to actually do anything here.
      break;
    case WM_CLIPBOARDUPDATE:
      delegate_->on_change.Emit(delegate_);
      break;
    case WM_DRAWCLIPBOARD:
      break;
    case WM_DESTROY:
      break;
    case WM_CHANGECBCHAIN:
      break;
    default:
      return false;
    }
    *result = 0;
    return true;
  }

  Clipboard* delegate_;
};

template <typename charT>
HGLOBAL CreateGlobalData(const std::basic_string<charT>& str) {
  HGLOBAL data =
    ::GlobalAlloc(GMEM_MOVEABLE, ((str.size() + 1) * sizeof(charT)));
  if (data) {
    charT* raw_data = static_cast<charT*>(::GlobalLock(data));
    memcpy(raw_data, str.data(), str.size() * sizeof(charT));
    raw_data[str.size()] = '\0';
    ::GlobalUnlock(data);
  }
  return data;
}

void FreeData(unsigned int format, HANDLE data) {
  if (format == CF_BITMAP)
    ::DeleteObject(static_cast<HBITMAP>(data));
  else
    ::GlobalFree(data);
}

template <typename StringType>
void TrimAfterNull(StringType* result) {
  // Text copied to the clipboard may explicitly contain null characters that
  // should be ignored, depending on the application that does the copying.
  constexpr typename StringType::value_type kNull = 0;
  size_t pos = result->find_first_of(kNull);
  if (pos != StringType::npos)
    result->resize(pos);
}

}  // namespace

// The implementation of clipboard.
class ClipboardImpl {
 public:
  using Data = Clipboard::Data;

  explicit ClipboardImpl(Clipboard* delegate) : clipboard_owner_(delegate) {}
  ~ClipboardImpl() {}

  bool IsDataAvailable(Data::Type type) const {
    int cf_type = ToCFType(type);
    if (cf_type < 0)
      return false;
    return ::IsClipboardFormatAvailable(cf_type);
  }

  Data GetData(Data::Type type) const {
    ScopedClipboard clipboard;
    if (!clipboard.Acquire(clipboard_owner_.hwnd()))
      return Data();

    switch (type) {
      case Data::Type::Text: {
        std::wstring result;
        if (!ReadText(&result))
          return Data();
        return Data(Data::Type::Text, base::WideToUTF8(result));
      }
      case Data::Type::HTML: {
        std::string result;
        if (!ReadHTML(&result))
          return Data();
        return Data(Data::Type::HTML, std::move(result));
      }
      case Data::Type::Image: {
        Image* image = ReadImage();
        return image ? Data(image) : Data();
      }
      case Data::Type::FilePaths: {
        std::vector<base::FilePath> result;
        if (!ReadFilePaths(&result))
          return Data();
        return Data(std::move(result));
      }
      case Data::Type::None:
        return Data();
    }
    NOTREACHED() << "Invalid type: " << static_cast<int>(type);
    return Data();
  }

  void SetData(std::vector<Data> objects) {
    ScopedClipboard clipboard;
    if (!clipboard.Acquire(clipboard_owner_.hwnd()))
      return;

    ::EmptyClipboard();

    for (Data& data : objects) {
      switch (data.type()) {
        case Data::Type::Text:
          WriteToClipboard(CF_UNICODETEXT,
                           CreateGlobalData(base::UTF8ToWide(data.str())));
          break;
        case Data::Type::HTML:
          WriteToClipboard(GetHTMLFormat(),
                           CreateGlobalData(HtmlToCFHtml(data.str(), "")));
          break;
        case Data::Type::Image:
          WriteToClipboard(CF_BITMAP, GetBitmapFromImage(data.image()));
          break;
        case Data::Type::FilePaths: {
          STGMEDIUM* storage = GetStorageForFileNames(data.file_paths());
          if (storage) {
            WriteToClipboard(CF_HDROP, storage->hGlobal);
            delete storage;
          }
          break;
        }
        case Data::Type::None:
          break;
        default:
          NOTREACHED() << "Invalid type: " << static_cast<int>(data.type());
      }
    }
  }

  void StartWatching() {
    ::AddClipboardFormatListener(clipboard_owner_.hwnd());
  }

  void StopWatching() {
    ::RemoveClipboardFormatListener(clipboard_owner_.hwnd());
  }

 private:
  // Safely write to system clipboard. Free |handle| on failure.
  void WriteToClipboard(UINT format, HANDLE handle) {
    if (handle && !::SetClipboardData(format, handle)) {
      DCHECK(ERROR_CLIPBOARD_NOT_OPEN != GetLastError());
      FreeData(format, handle);
    }
  }

  bool ReadText(std::wstring* result) const {
    HANDLE data = ::GetClipboardData(CF_UNICODETEXT);
    if (!data)
      return false;

    result->assign(static_cast<const wchar_t*>(::GlobalLock(data)),
                   ::GlobalSize(data) / sizeof(wchar_t));
    ::GlobalUnlock(data);
    TrimAfterNull(result);
    return true;
  }

  bool ReadHTML(std::string* result) const {
    HANDLE data = ::GetClipboardData(GetHTMLFormat());
    if (!data)
      return false;

    std::string cf_html(static_cast<const char*>(::GlobalLock(data)),
                        ::GlobalSize(data));
    ::GlobalUnlock(data);
    TrimAfterNull(&cf_html);

    CFHtmlToHtml(cf_html, result, nullptr);
    return true;
  }

  Image* ReadImage() const {
    BITMAPINFO* bitmap = static_cast<BITMAPINFO*>(::GetClipboardData(CF_DIBV5));
    if (!bitmap)
      return nullptr;

    // For more information on BITMAPINFOHEADER and biBitCount definition,
    // see https://docs.microsoft.com/en-us/previous-versions//dd183376(v=vs.85)
    int color_table_length = 0;
    switch (bitmap->bmiHeader.biBitCount) {
      case 1:
      case 4:
      case 8:
        color_table_length = bitmap->bmiHeader.biClrUsed
            ? bitmap->bmiHeader.biClrUsed
            : 1 << bitmap->bmiHeader.biBitCount;
        break;
      case 16:
      case 32:
        if (bitmap->bmiHeader.biCompression == BI_BITFIELDS)
          color_table_length = 3;
        break;
      case 24:
        break;
      default:
        NOTREACHED();
    }
    void* bits = reinterpret_cast<char*>(bitmap) +
                 bitmap->bmiHeader.biSize +
                 color_table_length * sizeof(RGBQUAD);

    Gdiplus::Bitmap* ret = Gdiplus::Bitmap::FromBITMAPINFO(bitmap, bits);
    if (!ret)
      return nullptr;
    return new Image(ret);
  }

  bool ReadFilePaths(std::vector<base::FilePath>* result) const {
    HANDLE data = ::GetClipboardData(CF_HDROP);
    if (!data)
      return false;

    HDROP drop = static_cast<HDROP>(::GlobalLock(data));
    GetFilePathsFromHDrop(drop, result);
    ::GlobalUnlock(data);
    return true;
  }

  ClipboardWindow clipboard_owner_;
};

///////////////////////////////////////////////////////////////////////////////
// Public Clipboard API implementation.

NativeClipboard Clipboard::PlatformCreate(Type type) {
  return new ClipboardImpl(this);
}

void Clipboard::PlatformDestroy() {
  delete clipboard_;
}

bool Clipboard::IsDataAvailable(Data::Type type) const {
  return clipboard_->IsDataAvailable(type);
}

Clipboard::Data Clipboard::GetData(Data::Type type) const {
  return clipboard_->GetData(type);
}

void Clipboard::SetData(std::vector<Data> objects) {
  clipboard_->SetData(std::move(objects));
}

void Clipboard::PlatformStartWatching() {
  clipboard_->StartWatching();
}

void Clipboard::PlatformStopWatching() {
  clipboard_->StopWatching();
}

}  // namespace nu
