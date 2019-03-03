// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/font.h"

#include <algorithm>

#include "base/strings/string_util.h"
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
  hfont_ = ::CreateFontIndirectW(&(metrics.lfMessageFont));

  base::win::ScopedGetDC screen_dc(NULL);
  ScopedSetMapMode mode(screen_dc, MM_TEXT);
  base::win::ScopedSelectObject scoped_font(screen_dc, hfont_);
  ::GetTextMetrics(screen_dc, &fm);
  float font_size = std::max<float>(1.f, fm.tmHeight - fm.tmInternalLeading);

  // Converting pixel size to point.
  font_size = font_size / ::GetDeviceCaps(screen_dc, LOGPIXELSX) * 72.f;

  // Create default font.
  font_ = new Gdiplus::Font(metrics.lfMessageFont.lfFaceName, font_size,
                            Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
}

Font::Font(const std::string& name, float size, Weight weight, Style style) {
  int font_style = Gdiplus::FontStyleRegular;
  if (weight >= Font::Weight::Bold)
    font_style |= Gdiplus::FontStyleBold;
  if (style == Font::Style::Italic)
    font_style |= Gdiplus::FontStyleItalic;
  font_ = new Gdiplus::Font(base::UTF8ToUTF16(name).c_str(),
                            // Converting DPI-aware pixel size to point.
                            size * 72.f / 96.f,
                            font_style,
                            Gdiplus::UnitPoint);
}

Font::~Font() {
  delete font_;
  if (hfont_)
    ::DeleteObject(hfont_);
}

std::string Font::GetName() const {
  return base::UTF16ToUTF8(GetName16());
}

float Font::GetSize() const {
  return font_->GetSize() / 72.f * 96.f;
}

Font::Weight Font::GetWeight() const {
  int style = font_->GetStyle();
  if (style & Gdiplus::FontStyleBold)
    return Weight::Bold;
  return Weight::Normal;
}

Font::Style Font::GetStyle() const {
  int style = font_->GetStyle();
  if (style & Gdiplus::FontStyleItalic)
    return Style::Italic;
  return Style::Normal;
}

NativeFont Font::GetNative() const {
  return font_;
}

const std::wstring& Font::GetName16() const {
  if (font_family_.empty()) {
    Gdiplus::FontFamily family;
    font_->GetFamily(&family);
    family.GetFamilyName(base::WriteInto(&font_family_, LF_FACESIZE));
    font_family_.resize(wcslen(font_family_.data()));
  }
  return font_family_;
}

HFONT Font::GetHFONT(HWND hwnd) const {
  if (!hfont_) {
    base::win::ScopedGetDC dc(hwnd);
    Gdiplus::Graphics context(dc);
    LOGFONTW logfont;
    font_->GetLogFontW(&context, &logfont);
    hfont_ = ::CreateFontIndirect(&logfont);
  }
  return hfont_;
}

}  // namespace nu
