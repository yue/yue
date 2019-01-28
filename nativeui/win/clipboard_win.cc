// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/clipboard.h"

#include "base/strings/utf_string_conversions.h"
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

 private:
  BOOL must_revert_;
  DISALLOW_COPY_AND_ASSIGN(AnonymousImpersonator);
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
  ClipboardWindow() : Win32Window(L"", HWND_MESSAGE, 0) {}

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
  ClipboardImpl() {}
  ~ClipboardImpl() {}

  void Clear() {
    ScopedClipboard clipboard;
    if (!clipboard.Acquire(clipboard_owner_.hwnd()))
      return;

    ::EmptyClipboard();
  }

  void SetText(base::string16 text) {
    ScopedClipboard clipboard;
    if (!clipboard.Acquire(clipboard_owner_.hwnd()))
      return;

    ::EmptyClipboard();

    HGLOBAL glob = CreateGlobalData(text);
    WriteToClipboard(CF_UNICODETEXT, glob);
  }

  base::string16 GetText() const {
    base::string16 result;

    ScopedClipboard clipboard;
    if (!clipboard.Acquire(clipboard_owner_.hwnd()))
      return result;

    HANDLE data = ::GetClipboardData(CF_UNICODETEXT);
    if (!data)
      return result;

    result.assign(static_cast<const base::char16*>(::GlobalLock(data)),
                  ::GlobalSize(data) / sizeof(base::char16));
    ::GlobalUnlock(data);
    TrimAfterNull(&result);

    return result;
  }

 private:
  // Safely write to system clipboard. Free |handle| on failure.
  void WriteToClipboard(unsigned int format, HANDLE handle) {
    if (handle && !::SetClipboardData(format, handle)) {
      DCHECK(ERROR_CLIPBOARD_NOT_OPEN != GetLastError());
      FreeData(format, handle);
    }
  }

  ClipboardWindow clipboard_owner_;

  DISALLOW_COPY_AND_ASSIGN(ClipboardImpl);
};

///////////////////////////////////////////////////////////////////////////////
// Public Clipboard API implementation.

NativeClipboard Clipboard::PlatformCreate(Type type) {
  return new ClipboardImpl;
}

void Clipboard::PlatformDestroy() {
  delete clipboard_;
}

void Clipboard::Clear() {
  clipboard_->Clear();
}

void Clipboard::SetText(const std::string& text) {
  clipboard_->SetText(base::UTF8ToUTF16(text));
}

std::string Clipboard::GetText() const {
  return base::UTF16ToUTF8(clipboard_->GetText());
}

}  // namespace nu
