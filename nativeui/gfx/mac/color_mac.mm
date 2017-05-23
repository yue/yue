// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/color.h"

#import <Cocoa/Cocoa.h>

namespace nu {

NSColor* Color::ToNSColor() const {
  return [NSColor colorWithCalibratedRed:r() / 255.0
                                   green:g() / 255.0
                                    blue:b() / 255.0
                                   alpha:a() / 255.0];
}

}  // namespace nu
