// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/screen.h"

#import <Cocoa/Cocoa.h>

namespace nu {

float GetScaleFactor() {
  NSScreen* screen = [[NSScreen screens] firstObject];
  if (!screen)  // no screen, can it happen?
    return 1.f;
  return [screen backingScaleFactor];
}

}  // namespace nu
