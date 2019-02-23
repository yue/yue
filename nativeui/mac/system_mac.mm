// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/system.h"

#import <Cocoa/Cocoa.h>

#include "base/logging.h"
#include "nativeui/gfx/geometry/safe_integer_conversions.h"

namespace nu {

Color System::GetColor(System::Color name) {
  NSColor* color;
  switch (name) {
    case System::Color::Text:
      color = [NSColor textColor];
      break;
    case System::Color::DisabledText:
      color = [NSColor disabledControlTextColor];
      break;
    default:
      NOTREACHED() << "Unkown color name: " << static_cast<int>(name);
      return nu::Color();
  }

  CGFloat red, green, blue, alpha;
  color = [color colorUsingColorSpace:[NSColorSpace deviceRGBColorSpace]];
  [color getRed:&red green:&green blue:&blue alpha:&alpha];
  return nu::Color(ToRoundedInt(255. * alpha),
                   ToRoundedInt(255. * red),
                   ToRoundedInt(255. * green),
                   ToRoundedInt(255. * blue));
}

}  // namespace nu
