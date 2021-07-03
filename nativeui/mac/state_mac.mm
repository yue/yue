// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/state.h"

#include "nativeui/mac/events_handler.h"
#include "third_party/yoga/Yoga.h"

namespace nu {

void State::PlatformInit() {
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

  // Disable TouchBar when the executable is not inside an app bundle, otherwise
  // Cocoa would just crash.
  if (![[NSBundle mainBundle] bundleIdentifier]) {
    NSDictionary* defaults = [NSDictionary
        dictionaryWithObject:[NSNumber numberWithBool:NO]
                      forKey:@"NSFunctionBarAPIEnabled"];
    [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];
  }

  YGConfigSetPointScaleFactor(yoga_config(),
                              [NSScreen mainScreen].backingScaleFactor);
}

}  // namespace nu
