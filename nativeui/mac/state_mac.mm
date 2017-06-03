// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/state.h"

#include "nativeui/mac/events_handler.h"
#include "third_party/yoga/yoga/Yoga.h"

namespace nu {

void State::PlatformInit() {
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

  YGConfigSetPointScaleFactor(yoga_config(),
                              [NSScreen mainScreen].backingScaleFactor);
}

}  // namespace nu
