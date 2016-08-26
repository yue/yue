// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/text.h"

#include "base/win/scoped_gdi_object.h"
#include "base/win/scoped_hdc.h"
#include "nativeui/gfx/win/gdiplus.h"

namespace nu {

SizeF MeasureText(const Font& font, const String& text) {
  base::win::ScopedCreateDC dc(CreateCompatibleDC(NULL));

  Gdiplus::Graphics graphics(dc.Get());
  Gdiplus::FontFamily gdi_family(font.family.c_str());
  Gdiplus::Font gdi_font(&gdi_family, font.size,
                         Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

  Gdiplus::RectF rect;
  Gdiplus::StringFormat fomart(Gdiplus::StringFormat::GenericDefault());
  graphics.MeasureString(text.c_str(), static_cast<int>(text.length()),
                         &gdi_font, Gdiplus::PointF(0., 0.), &fomart, &rect);
  return SizeF(rect.Width, rect.Height);
}

}  // namespace nu
