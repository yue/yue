// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/state.h"

#import <Cocoa/Cocoa.h>

namespace nu {

void State::PlatformInit() {
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
}

void State::PlatformDestroy() {
}

}  // namespace nu
