// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/system.h"

#include "base/logging.h"

namespace nu {

Color System::GetColor(System::Color name) {
  int index = 0;
  switch (name) {
    case System::Color::Text:
      index = COLOR_WINDOWTEXT;
      break;
    case System::Color::DisabledText:
      index = COLOR_GRAYTEXT;
      break;
    default:
      NOTREACHED() << "Unkown color name: " << static_cast<int>(name);
      return nu::Color();
  }

  DWORD color = ::GetSysColor(index);
  return nu::Color(GetRValue(color), GetGValue(color), GetBValue(color));
}

}  // namespace nu
