// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/text_win.h"

#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_gdi_object.h"
#include "base/win/scoped_hdc.h"
#include "nativeui/gfx/win/gdiplus.h"
#include "nativeui/view.h"
#include "nativeui/win/view_win.h"

namespace nu {

SizeF MeasureText(base::StringPiece text, Font* font) {
  return MeasureText(base::UTF8ToUTF16(text), font);
}

SizeF MeasureText(const base::string16& text, Font* font) {
  base::win::ScopedGetDC dc(NULL);
  return MeasureText(dc, text, font);
}

SizeF MeasureText(HDC dc, const base::string16& text, Font* font) {
  Gdiplus::Graphics graphics(dc);
  Gdiplus::RectF rect;
  graphics.MeasureString(text.c_str(), static_cast<int>(text.length()),
                         font->GetNative(), Gdiplus::PointF(0., 0.), &rect);
  return SizeF(rect.Width, rect.Height);
}

}  // namespace nu
