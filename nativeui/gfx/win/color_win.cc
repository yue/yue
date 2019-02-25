// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/color.h"

namespace nu {

COLORREF Color::ToCOLORREF() const {
  return RGB(r(), g(), b());
}

D2D1_COLOR_F Color::ToD2D1Color() const {
  return {r() / 255.f, g() / 255.f, b() / 255.f, a() / 255.f};
}

}  // namespace nu
