// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/accelerator.h"

#include <vector>

#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"

namespace nu {

namespace {

// Return key code of the char, and also determine whether the SHIFT key is
// pressed.
KeyboardCode KeyboardCodeFromCharCode(base::char16 c, bool* shifted) {
  c = base::ToLowerASCII(c);
  *shifted = false;
  switch (c) {
    case 0x08: return VKEY_BACK;
    case 0x7F: return VKEY_DELETE;
    case 0x09: return VKEY_TAB;
    case 0x0D: return VKEY_RETURN;
    case 0x1B: return VKEY_ESCAPE;
    case ' ': return VKEY_SPACE;

    case 'a': return VKEY_A;
    case 'b': return VKEY_B;
    case 'c': return VKEY_C;
    case 'd': return VKEY_D;
    case 'e': return VKEY_E;
    case 'f': return VKEY_F;
    case 'g': return VKEY_G;
    case 'h': return VKEY_H;
    case 'i': return VKEY_I;
    case 'j': return VKEY_J;
    case 'k': return VKEY_K;
    case 'l': return VKEY_L;
    case 'm': return VKEY_M;
    case 'n': return VKEY_N;
    case 'o': return VKEY_O;
    case 'p': return VKEY_P;
    case 'q': return VKEY_Q;
    case 'r': return VKEY_R;
    case 's': return VKEY_S;
    case 't': return VKEY_T;
    case 'u': return VKEY_U;
    case 'v': return VKEY_V;
    case 'w': return VKEY_W;
    case 'x': return VKEY_X;
    case 'y': return VKEY_Y;
    case 'z': return VKEY_Z;

    case ')': *shifted = true; case '0': return VKEY_0;
    case '!': *shifted = true; case '1': return VKEY_1;
    case '@': *shifted = true; case '2': return VKEY_2;
    case '#': *shifted = true; case '3': return VKEY_3;
    case '$': *shifted = true; case '4': return VKEY_4;
    case '%': *shifted = true; case '5': return VKEY_5;
    case '^': *shifted = true; case '6': return VKEY_6;
    case '&': *shifted = true; case '7': return VKEY_7;
    case '*': *shifted = true; case '8': return VKEY_8;
    case '(': *shifted = true; case '9': return VKEY_9;

    case ':': *shifted = true; case ';': return VKEY_OEM_1;
    case '+': *shifted = true; case '=': return VKEY_OEM_PLUS;
    case '<': *shifted = true; case ',': return VKEY_OEM_COMMA;
    case '_': *shifted = true; case '-': return VKEY_OEM_MINUS;
    case '>': *shifted = true; case '.': return VKEY_OEM_PERIOD;
    case '?': *shifted = true; case '/': return VKEY_OEM_2;
    case '~': *shifted = true; case '`': return VKEY_OEM_3;
    case '{': *shifted = true; case '[': return VKEY_OEM_4;
    case '|': *shifted = true; case '\\': return VKEY_OEM_5;
    case '}': *shifted = true; case ']': return VKEY_OEM_6;
    case '"': *shifted = true; case '\'': return VKEY_OEM_7;

    default: return VKEY_UNKNOWN;
  }
}

// Return key code represented by |str|.
KeyboardCode KeyboardCodeFromKeyIdentifier(const std::string& s,
                                           bool* shifted) {
  std::string str = base::ToLowerASCII(s);
  if (str == "ctrl" || str == "control") {
    return VKEY_CONTROL;
  } else if (str == "super" || str == "cmd" || str == "command" ||
             str == "meta") {
    return VKEY_COMMAND;
  } else if (str == "commandorcontrol" || str == "cmdorctrl") {
#if defined(OS_MACOSX)
    return VKEY_COMMAND;
#else
    return VKEY_CONTROL;
#endif
  } else if (str == "alt" || str == "option") {
    return VKEY_MENU;
  } else if (str == "shift") {
    return VKEY_SHIFT;
  } else if (str == "plus") {
    *shifted = true;
    return VKEY_OEM_PLUS;
  } else if (str == "tab") {
    return VKEY_TAB;
  } else if (str == "space") {
    return VKEY_SPACE;
  } else if (str == "backspace") {
    return VKEY_BACK;
  } else if (str == "delete") {
    return VKEY_DELETE;
  } else if (str == "insert") {
    return VKEY_INSERT;
  } else if (str == "enter" || str == "return") {
    return VKEY_RETURN;
  } else if (str == "up") {
    return VKEY_UP;
  } else if (str == "down") {
    return VKEY_DOWN;
  } else if (str == "left") {
    return VKEY_LEFT;
  } else if (str == "right") {
    return VKEY_RIGHT;
  } else if (str == "home") {
    return VKEY_HOME;
  } else if (str == "end") {
    return VKEY_END;
  } else if (str == "pageup") {
    return VKEY_PRIOR;
  } else if (str == "pagedown") {
    return VKEY_NEXT;
  } else if (str == "esc" || str == "escape") {
    return VKEY_ESCAPE;
  } else if (str == "volumemute") {
    return VKEY_VOLUME_MUTE;
  } else if (str == "volumeup") {
    return VKEY_VOLUME_UP;
  } else if (str == "volumedown") {
    return VKEY_VOLUME_DOWN;
  } else if (str == "medianexttrack") {
    return VKEY_MEDIA_NEXT_TRACK;
  } else if (str == "mediaprevioustrack") {
    return VKEY_MEDIA_PREV_TRACK;
  } else if (str == "mediastop") {
    return VKEY_MEDIA_STOP;
  } else if (str == "mediaplaypause") {
    return VKEY_MEDIA_PLAY_PAUSE;
  } else if (str == "printscreen") {
    return VKEY_SNAPSHOT;
  } else if (str.size() > 1 && str[0] == 'f') {
    // F1 - F24.
    int n;
    if (base::StringToInt(str.c_str() + 1, &n) && n > 0 && n < 25) {
      return static_cast<KeyboardCode>(VKEY_F1 + n - 1);
    } else {
      LOG(WARNING) << str << "is not available on keyboard";
      return VKEY_UNKNOWN;
    }
  } else {
    if (str.size() > 2)
      LOG(WARNING) << "Invalid accelerator token: " << str;
    return VKEY_UNKNOWN;
  }
}

KeyboardCode KeyboardCodeFromStr(const std::string& str, bool* shifted) {
  if (str.size() == 1)
    return KeyboardCodeFromCharCode(str[0], shifted);
  else
    return KeyboardCodeFromKeyIdentifier(str, shifted);
}

}  // namespace

// static
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

bool Accelerator::operator <(const Accelerator& rhs) const {
  if (key_code_ != rhs.key_code_)
    return key_code_ < rhs.key_code_;
  return modifiers_ < rhs.modifiers_;
}

bool Accelerator::operator ==(const Accelerator& rhs) const {
  return (key_code_ == rhs.key_code_) && (modifiers_ == rhs.modifiers_);
}

bool Accelerator::operator !=(const Accelerator& rhs) const {
  return !(*this == rhs);
}

Accelerator::Accelerator(KeyboardCode code, int modifiers)
    : key_code_(code), modifiers_(modifiers) {}

#if defined(OS_WIN)
std::string Accelerator::GetShortcutText() const {
  std::string shortcut;
  if (modifiers_ & MASK_SHIFT)
    shortcut += "Shift+";
  if (modifiers_ & MASK_CONTROL)
    shortcut += "Control+";
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
      shortcut += "Delete";
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
