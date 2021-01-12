// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/color.h"

#include "base/notreached.h"

namespace nu {

// static
Color Color::Get(Name name) {
  int index = 0;
  switch (name) {
    case Color::Name::Text:
      index = COLOR_WINDOWTEXT;
      break;
    case Color::Name::DisabledText:
      index = COLOR_GRAYTEXT;
      break;
    case Color::Name::Control:
      index = COLOR_BTNFACE;
      break;
    case Color::Name::WindowBackground:
      index = COLOR_WINDOW;
      break;
    default:
      NOTREACHED();
  }
  DWORD color = ::GetSysColor(index);
  return Color(GetRValue(color), GetGValue(color), GetBValue(color));
}

COLORREF Color::ToCOLORREF() const {
  return RGB(r(), g(), b());
}

}  // namespace nu
