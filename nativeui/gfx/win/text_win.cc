// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/text_win.h"

#include "base/win/scoped_hdc.h"
#include "nativeui/gfx/win/gdiplus.h"

namespace nu {

SizeF MeasureText(const base::StringPiece16& text, Font* font) {
  base::win::ScopedGetDC dc(NULL);
  return MeasureText(dc, text, font);
}

SizeF MeasureText(HDC dc, const base::StringPiece16& text, Font* font) {
  Gdiplus::Graphics graphics(dc);
  Gdiplus::RectF rect;
  graphics.MeasureString(text.data(), static_cast<int>(text.length()),
                         font->GetNative(), Gdiplus::PointF(0., 0.), &rect);
  return SizeF(rect.Width, rect.Height);
}

}  // namespace nu
