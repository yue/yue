// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_EVENTS_KEYBOARD_CODES_H_
#define NATIVEUI_EVENTS_KEYBOARD_CODES_H_

#include "build/build_config.h"

#if defined(OS_MACOSX)
#include "nativeui/events/keyboard_codes_mac.h"
#endif

namespace nu {

enum KeyboardModifier {
  MODIFIER_NONE    = 0,
  MODIFIER_SHIFT   = 1 << 1,
  MODIFIER_CONTROL = 1 << 2,
  MODIFIER_ALT     = 1 << 3,
  MODIFIER_COMMAND = 1 << 4,
  MODIFIER_ALTGR   = 1 << 5,
};

}  // namespace nu

#endif  // NATIVEUI_EVENTS_KEYBOARD_CODES_H_
