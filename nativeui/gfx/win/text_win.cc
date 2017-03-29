// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/text_win.h"

#include "base/win/scoped_gdi_object.h"
#include "base/win/scoped_hdc.h"
#include "nativeui/gfx/win/gdiplus.h"
#include "nativeui/view.h"
#include "nativeui/win/view_win.h"

namespace nu {

SizeF MeasureText(Font* font, const String& text) {
  base::win::ScopedGetDC dc(NULL);
  return MeasureText(dc, font, text);
}

SizeF MeasureText(HDC dc, Font* font, const String& text) {
  Gdiplus::Graphics graphics(dc);
  Gdiplus::RectF rect;
  Gdiplus::StringFormat fomart(Gdiplus::StringFormat::GenericDefault());
  graphics.MeasureString(text.c_str(), static_cast<int>(text.length()),
                         font->GetNative(), Gdiplus::PointF(0., 0.),
                         &fomart, &rect);
  return SizeF(rect.Width, rect.Height);
}

}  // namespace nu
