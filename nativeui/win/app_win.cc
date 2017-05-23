// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/app.h"

namespace nu {

Color App::PlatformGetColor(ThemeColor name) {
  int index = 0;
  if (name == ThemeColor::Text)
    index = COLOR_WINDOWTEXT;
  DWORD color = ::GetSysColor(index);
  return Color(GetRValue(color), GetGValue(color), GetBValue(color));
}

}  // namespace nu
