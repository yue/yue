// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_EVENTS_KEYBOARD_CODES_H_
#define NATIVEUI_EVENTS_KEYBOARD_CODES_H_

#include "build/build_config.h"

#if defined(OS_MAC)
#include "nativeui/events/mac/keyboard_codes_mac.h"
#elif defined(OS_LINUX)
#include "nativeui/events/gtk/keyboard_codes_gtk.h"
#elif defined(OS_WIN)
#include "nativeui/events/win/keyboard_codes_win.h"
#endif

#endif  // NATIVEUI_EVENTS_KEYBOARD_CODES_H_
