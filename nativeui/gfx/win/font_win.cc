// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/font.h"

#include "base/win/scoped_gdi_object.h"
#include "base/win/scoped_hdc.h"
#include "nativeui/gfx/win/gdiplus.h"

namespace nu {

Font::Font() : family(L"Tahoma"), size(24) {
}

Font GetDefaultFont() {
  base::win::ScopedCreateDC dc(CreateCompatibleDC(NULL));
  base::win::ScopedHFONT hfont(
      reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT)));

  Gdiplus::Font font(dc.Get(), hfont.get());
  Gdiplus::FontFamily family;
  if (font.GetFamily(&family) != Gdiplus::Ok)
    return Font();

  wchar_t buffer[LF_FACESIZE] = {0};
  family.GetFamilyName(buffer);
  return Font(buffer, font.GetSize());
}

}  // namespace nu
