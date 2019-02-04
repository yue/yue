// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/dragging_info_win.h"

#include <string>
#include <utility>
#include <vector>

#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_hglobal.h"
#include "nativeui/gfx/win/gdiplus.h"
#include "nativeui/win/drag_drop/clipboard_util.h"

namespace nu {

namespace {

bool GetFormatEtc(Clipboard::Data::Type type, FORMATETC* format) {
  int cf_type = ToCFType(type);
  if (cf_type < 0)
    return false;
  format->cfFormat = static_cast<CLIPFORMAT>(cf_type);
  format->dwAspect = DVASPECT_CONTENT;
  format->lindex = -1;
  format->tymed = TYMED_HGLOBAL;
  return true;
}

}  // namespace

DraggingInfoWin::DraggingInfoWin(IDataObject* data, int effect)
    : DraggingInfo(effect), data_(data) {}

DraggingInfoWin::~DraggingInfoWin() {}

bool DraggingInfoWin::IsDataAvailable(Data::Type type) const {
  FORMATETC format = {0};
  if (!GetFormatEtc(type, &format))
    return false;
  return SUCCEEDED(data_->QueryGetData(&format));
}

Clipboard::Data DraggingInfoWin::GetData(Data::Type type) const {
  FORMATETC format = {0};
  if (!GetFormatEtc(type, &format))
    return Data();

  STGMEDIUM medium;
  if (FAILED(data_->GetData(&format, &medium)))
    return Data();

  Data ret;
  switch (type) {
    case Data::Type::Text: {
      base::win::ScopedHGlobal<wchar_t*> data(medium.hGlobal);
      base::string16 result(data.get());
      ret = Data(Data::Type::Text, base::UTF16ToUTF8(result));
      break;
    }
    case Data::Type::HTML: {
      base::win::ScopedHGlobal<char*> data(medium.hGlobal);
      std::string result;
      CFHtmlToHtml(data.get(), &result, nullptr);
      ret = Data(Data::Type::HTML, std::move(result));
      break;
    }
    case Data::Type::Image: {
      base::win::ScopedHGlobal<BITMAPINFO*> data(medium.hGlobal);
      if (data.get()) {
        Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromBITMAPINFO(
            data.get(), data.get() + sizeof(BITMAPINFO));
        if (bitmap)
          ret = Data(new Image(bitmap));
      }
      break;
    }
    case Data::Type::FilePaths: {
      base::win::ScopedHGlobal<HDROP> hdrop(medium.hGlobal);
      if (hdrop.get()) {
        std::vector<base::FilePath> result;
        GetFilePathsFromHDrop(hdrop.get(), &result);
        ret = Data(std::move(result));
      }
      break;
    }
    default:
      NOTREACHED() << "Invalid clipboard data type: " << static_cast<int>(type);
  }

  ReleaseStgMedium(&medium);
  return std::move(ret);
}

}  // namespace nu
