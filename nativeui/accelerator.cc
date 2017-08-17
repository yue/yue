// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/accelerator.h"

#include <vector>

#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "nativeui/events/event.h"
#include "nativeui/events/keyboard_code_conversion.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

namespace nu {

Accelerator::Accelerator()
    : key_code_(VKEY_UNKNOWN), modifiers_(0) {
}

Accelerator::Accelerator(KeyboardCode key_code, int modifiers)
    : key_code_(key_code), modifiers_(modifiers) {}

Accelerator::Accelerator(const KeyEvent& event)
    : key_code_(event.key), modifiers_(event.modifiers) {}

Accelerator::Accelerator(const std::string& description)
    : key_code_(VKEY_UNKNOWN), modifiers_(0) {
  if (!base::IsStringASCII(description))
    return;

  std::vector<std::string> tokens = base::SplitString(
     description, "+", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);

  // Now, parse it into an accelerator.
  for (const auto& token : tokens) {
    bool shifted = false;
    KeyboardCode code = KeyboardCodeFromStr(token, &shifted);
    if (shifted)
      modifiers_ |= MASK_SHIFT;
    switch (code) {
      // The token can be a modifier.
      case VKEY_SHIFT:
        modifiers_ |= MASK_SHIFT;
        break;
      case VKEY_CONTROL:
        modifiers_ |= MASK_CONTROL;
        break;
      case VKEY_MENU:
        modifiers_ |= MASK_ALT;
        break;
      case VKEY_COMMAND:
        modifiers_ |= MASK_META;
        break;
      // Or it is a normal key.
      default:
        key_code_ = code;
    }
  }
}

bool Accelerator::operator< (const Accelerator& rhs) const {
  if (key_code_ != rhs.key_code_)
    return key_code_ < rhs.key_code_;
  return modifiers_ < rhs.modifiers_;
}

bool Accelerator::operator== (const Accelerator& rhs) const {
  return (key_code_ == rhs.key_code_) && (modifiers_ == rhs.modifiers_);
}

bool Accelerator::operator!= (const Accelerator& rhs) const {
  return !(*this == rhs);
}

#if defined(OS_WIN)
std::string Accelerator::GetShortcutText() const {
  std::string shortcut;
  if (modifiers_ & MASK_SHIFT)
    shortcut += "Shift+";
  if (modifiers_ & MASK_CONTROL)
    shortcut += "Ctrl+";
  if (modifiers_ & MASK_ALT)
    shortcut += "Alt+";
  if (modifiers_ & MASK_META)
    shortcut += "Super+";

  switch (key_code_) {
    case VKEY_TAB:
      shortcut += "Tab";
      break;
    case VKEY_RETURN:
      shortcut += "Enter";
      break;
    case VKEY_ESCAPE:
      shortcut += "Escape";
      break;
    case VKEY_SPACE:
      shortcut += "Space";
      break;
    case VKEY_PRIOR:
      shortcut += "Pageup";
      break;
    case VKEY_NEXT:
      shortcut += "Pagedown";
      break;
    case VKEY_END:
      shortcut += "End";
      break;
    case VKEY_HOME:
      shortcut += "Home";
      break;
    case VKEY_INSERT:
      shortcut += "Insert";
      break;
    case VKEY_DELETE:
      shortcut += "Del";
      break;
    case VKEY_LEFT:
      shortcut += "Left";
      break;
    case VKEY_RIGHT:
      shortcut += "Right";
      break;
    case VKEY_UP:
      shortcut += "Up";
      break;
    case VKEY_DOWN:
      shortcut += "Down";
      break;
    case VKEY_BACK:
      shortcut += "Backspace";
      break;
    case VKEY_OEM_COMMA:
      shortcut += ",";
      break;
    case VKEY_OEM_PERIOD:
      shortcut += ".";
      break;
    case VKEY_MEDIA_NEXT_TRACK:
      shortcut += "NextTrack";
      break;
    case VKEY_MEDIA_PLAY_PAUSE:
      shortcut += "Play/Pause";
      break;
    case VKEY_MEDIA_PREV_TRACK:
      shortcut += "PreviousTrack";
      break;
    case VKEY_MEDIA_STOP:
      shortcut += "Stop";
      break;
    default:
      break;
  }

  if (shortcut.empty() || shortcut[shortcut.size() - 1] == '+') {
    wchar_t key;
    if (base::IsAsciiDigit(key_code_))
      key = static_cast<wchar_t>(key_code_);
    else
      key = LOWORD(::MapVirtualKeyW(key_code_, MAPVK_VK_TO_CHAR));
    shortcut += key;
  }

  return shortcut;
}
#endif

}  // namespace nu
