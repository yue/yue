// Copyright 2017 Cheng Zhao. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_EVENTS_KEYBOARD_CODE_CONVERSION_H_
#define NATIVEUI_EVENTS_KEYBOARD_CODE_CONVERSION_H_

#include <string>

#include "nativeui/events/keyboard_codes.h"
#include "nativeui/nativeui_export.h"

namespace nu {

// Convert a key code to string, using the values of KeyboardEvent.key:
// https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/key/Key_Values
NATIVEUI_EXPORT const char* KeyboardCodeToStr(KeyboardCode code);

// Convert a string to a key code represented by it.
NATIVEUI_EXPORT KeyboardCode KeyboardCodeFromStr(const std::string& str,
                                                 bool* shifted);

}  // namespace nu

#endif  // NATIVEUI_EVENTS_KEYBOARD_CODE_CONVERSION_H_
