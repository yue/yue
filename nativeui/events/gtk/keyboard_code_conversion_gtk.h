// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_EVENTS_GTK_KEYBOARD_CODE_CONVERSION_GTK_H_
#define NATIVEUI_EVENTS_GTK_KEYBOARD_CODE_CONVERSION_GTK_H_

#include "nativeui/events/gtk/keyboard_codes_gtk.h"

namespace nu {

KeyboardCode KeyboardCodeFromGdkKeyCode(int keyval);

}  // namespace nu

#endif  // NATIVEUI_EVENTS_GTK_KEYBOARD_CODE_CONVERSION_GTK_H_
