// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_ACCELERATOR_H_
#define NATIVEUI_ACCELERATOR_H_

#include <memory>
#include <string>

#include "nativeui/events/keyboard_codes.h"
#include "nativeui/nativeui_export.h"

namespace nu {

struct KeyEvent;

class NATIVEUI_EXPORT Accelerator {
 public:
  Accelerator();
  explicit Accelerator(KeyboardCode key_code, int modifiers);
  explicit Accelerator(const KeyEvent& event);
  explicit Accelerator(const std::string& description);
  ~Accelerator() = default;

  bool operator< (const Accelerator& rhs) const;
  bool operator== (const Accelerator& rhs) const;
  bool operator!= (const Accelerator& rhs) const;

#if defined(OS_WIN)
  std::string GetShortcutText() const;
#endif

  bool IsEmpty() const { return key_code_ == VKEY_UNKNOWN && modifiers_ == 0; }

  KeyboardCode GetKeyCode() const { return key_code_; }
  int GetModifiers() const { return modifiers_; }

 private:
  // The keycode (VK_...).
  KeyboardCode key_code_;

  // The state of the Shift/Ctrl/Alt keys.
  int modifiers_;
};

}  // namespace nu

#endif  // NATIVEUI_ACCELERATOR_H_
