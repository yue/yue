// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/color.h"

namespace nu {

// static
Color Color::Get(Name name) {
  int index = 0;
  if (name == Name::Text)
    index = COLOR_WINDOWTEXT;
  else if (name == Name::DisabledText)
    index = COLOR_GRAYTEXT;
  DWORD color = ::GetSysColor(index);
  return Color(GetRValue(color), GetGValue(color), GetBValue(color));
}

COLORREF Color::ToCOLORREF() const {
  return RGB(r(), g(), b());
}

}  // namespace nu
