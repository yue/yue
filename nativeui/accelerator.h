// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_ACCELERATOR_H_
#define NATIVEUI_ACCELERATOR_H_

#include <memory>
#include <string>

#include "nativeui/events/keyboard_codes.h"

namespace nu {

class Accelerator {
 public:
  explicit Accelerator(const std::string& description);
  explicit Accelerator(KeyboardCode code, int modifiers = 0);
  ~Accelerator() = default;

  bool empty() const { return key_code_ == VKEY_UNKNOWN && modifiers_ == 0; }

 private:
  // The keycode (VK_...).
  KeyboardCode key_code_;

  // The state of the Shift/Ctrl/Alt keys.
  int modifiers_;
};

}  // namespace nu

#endif  // NATIVEUI_ACCELERATOR_H_
