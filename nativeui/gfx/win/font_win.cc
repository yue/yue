// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/font.h"

#include <algorithm>

#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_gdi_object.h"
#include "base/win/scoped_hdc.h"
#include "base/win/scoped_select_object.h"
#include "nativeui/gfx/win/gdiplus.h"
#include "nativeui/gfx/win/scoped_set_map_mode.h"

namespace nu {

Font::Font() {
  // Receive default font family and size.
  NONCLIENTMETRICS metrics = {0};
  metrics.cbSize = sizeof(metrics);
  SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0);

  TEXTMETRIC fm;
  base::win::ScopedHFONT font(CreateFontIndirectW(&(metrics.lfMessageFont)));
  base::win::ScopedGetDC screen_dc(NULL);
  ScopedSetMapMode mode(screen_dc, MM_TEXT);
  base::win::ScopedSelectObject scoped_font(screen_dc, font.get());
  ::GetTextMetrics(screen_dc, &fm);
  float font_size = std::max<float>(1.0f, fm.tmHeight - fm.tmInternalLeading);

  // Create default font.
  font_ = new Gdiplus::Font(metrics.lfMessageFont.lfFaceName, font_size,
                            Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
}

Font::Font(base::StringPiece name, float size)
    : font_(new Gdiplus::Font(base::UTF8ToUTF16(name).c_str(),
                              // Converting DPI-aware pixel size to point.
                              size * 72.0f / 96.0f,
                              Gdiplus::FontStyleRegular,
                              Gdiplus::UnitPoint)) {
}

Font::~Font() {
}

std::string Font::GetName() const {
  Gdiplus::FontFamily family;
  font_->GetFamily(&family);
  WCHAR buffer[LF_FACESIZE] = {0};
  family.GetFamilyName(buffer);
  return base::UTF16ToUTF8(buffer);
}

int Font::GetSize() const {
  return font_->GetSize();
}

NativeFont Font::GetNative() const {
  return font_;
}

}  // namespace nu
