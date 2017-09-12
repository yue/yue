// Copyright 2016 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_EVENTS_MAC_KEYBOARD_CODE_CONVERSION_MAC_H_
#define NATIVEUI_EVENTS_MAC_KEYBOARD_CODE_CONVERSION_MAC_H_

#import <Cocoa/Cocoa.h>

#include "nativeui/events/mac/keyboard_codes_mac.h"
#include "nativeui/nativeui_export.h"

namespace nu {

// We use windows virtual keycodes throughout our keyboard event related code,
// including unit tests. But Mac uses a different set of virtual keycodes.
// This function converts a windows virtual keycode into Mac's virtual key code
// and corresponding unicode character. |flags| is the Cocoa modifiers mask
// such as NSControlKeyMask, NSShiftKeyMask, etc.
// When success, the corresponding Mac's virtual key code will be returned.
// |keyboard_character| is the corresponding keyboard character, suitable for
// use in -[NSEvent characters]. If NSShiftKeyMask appears in |flags|,
// |us_keyboard_shifted_character| is |keyboard_character| with a shift modifier
// applied using a US keyboard layout (otherwise unmodified).
// |us_keyboard_shifted_character| is suitable for -[NSEvent
// charactersIgnoringModifiers] (which ignores all modifiers except for shift).
// -1 will be returned if the keycode can't be converted.
// This function is mainly for simulating keyboard events in unit tests.
// See |KeyboardCodeFromNSEvent| for reverse conversion.
NATIVEUI_EXPORT int MacKeyCodeForWindowsKeyCode(
    KeyboardCode keycode,
    NSUInteger flags,
    unichar* us_keyboard_shifted_character,
    unichar* keyboard_character);

// This implementation cribbed from:
//   content/browser/render_host/input/web_input_event_builder_mac.mm
// Converts |event| into a |KeyboardCode|.  The mapping is not direct as the Mac
// has a different notion of key codes.
NATIVEUI_EXPORT KeyboardCode KeyboardCodeFromNSEvent(NSEvent* event);

}  // namespace nu

#endif  // NATIVEUI_EVENTS_MAC_KEYBOARD_CODE_CONVERSION_MAC_H_
