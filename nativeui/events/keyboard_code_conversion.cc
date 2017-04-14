// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/events/keyboard_code_conversion.h"

#include "base/strings/string_number_conversions.h"
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
  } else if (str == "plus" || str == "add") {
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
  } else if (str == "up" || str == "arrowup") {
    return VKEY_UP;
  } else if (str == "down" || str == "arrowdown") {
    return VKEY_DOWN;
  } else if (str == "left" || str == "arrowleft") {
    return VKEY_LEFT;
  } else if (str == "right" || str == "arrowright") {
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

}  // namespace

const char* KeyboardCodeToStr(KeyboardCode code) {
  switch (code) {
    case VKEY_CANCEL: return "Cancel";
    case VKEY_BACK: return "Backspace";
    case VKEY_TAB: return "Tab";
    case VKEY_BACKTAB: return "BackTab";
    case VKEY_CLEAR: return "Clear";
    case VKEY_RETURN: return "Enter";
    case VKEY_SHIFT: return "Shift";
    case VKEY_CONTROL: return "Control";
    case VKEY_MENU: return "Alt";
    case VKEY_PAUSE: return "Pause";
    case VKEY_CAPITAL: return "CapsLock";
#if defined(OS_LINUX)
    case VKEY_KANA: return "KanaMode";
#endif
    case VKEY_HANGUL: return "HangulMode";
#if defined(OS_LINUX)
    case VKEY_HANJA: return "HanjaMode";
#endif
    case VKEY_KANJI: return "KanjiMode";
    case VKEY_ESCAPE: return "Escape";
    case VKEY_SPACE: return "Space";
    case VKEY_PRIOR: return "PageUp";
    case VKEY_NEXT: return "PageDown";
    case VKEY_END: return "End";
    case VKEY_HOME: return "Home";
    case VKEY_LEFT: return "ArrowLeft";
    case VKEY_UP: return "ArrowUp";
    case VKEY_RIGHT: return "ArrowRight";
    case VKEY_DOWN: return "ArrowDown";
    case VKEY_SELECT: return "Select";
    case VKEY_PRINT: return "Print";
    case VKEY_EXECUTE: return "Execute";
    case VKEY_SNAPSHOT: return "Snapshot";
    case VKEY_INSERT: return "Insert";
    case VKEY_DELETE: return "Delete";
    case VKEY_HELP: return "Help";
    case VKEY_0: return "0";
    case VKEY_1: return "1";
    case VKEY_2: return "2";
    case VKEY_3: return "3";
    case VKEY_4: return "4";
    case VKEY_5: return "5";
    case VKEY_6: return "6";
    case VKEY_7: return "7";
    case VKEY_8: return "8";
    case VKEY_9: return "9";
    case VKEY_A: return "a";
    case VKEY_B: return "b";
    case VKEY_C: return "c";
    case VKEY_D: return "d";
    case VKEY_E: return "e";
    case VKEY_F: return "f";
    case VKEY_G: return "g";
    case VKEY_H: return "h";
    case VKEY_I: return "i";
    case VKEY_J: return "j";
    case VKEY_K: return "k";
    case VKEY_L: return "l";
    case VKEY_M: return "m";
    case VKEY_N: return "n";
    case VKEY_O: return "o";
    case VKEY_P: return "p";
    case VKEY_Q: return "q";
    case VKEY_R: return "r";
    case VKEY_S: return "s";
    case VKEY_T: return "t";
    case VKEY_U: return "u";
    case VKEY_V: return "v";
    case VKEY_W: return "w";
    case VKEY_X: return "x";
    case VKEY_Y: return "y";
    case VKEY_Z: return "z";
    case VKEY_COMMAND: return "Meta";
    case VKEY_RWIN: return "Meta";
    case VKEY_APPS: return "ContextMenu";
    case VKEY_NUMPAD0: return "0";
    case VKEY_NUMPAD1: return "1";
    case VKEY_NUMPAD2: return "2";
    case VKEY_NUMPAD3: return "3";
    case VKEY_NUMPAD4: return "4";
    case VKEY_NUMPAD5: return "5";
    case VKEY_NUMPAD6: return "6";
    case VKEY_NUMPAD7: return "7";
    case VKEY_NUMPAD8: return "8";
    case VKEY_NUMPAD9: return "9";
    case VKEY_MULTIPLY: return "Multiply";
    case VKEY_ADD: return "Add";
    case VKEY_SUBTRACT: return "Subtract";
    case VKEY_DECIMAL: return "Decimal";
    case VKEY_DIVIDE: return "Divide";
    case VKEY_F1: return "F1";
    case VKEY_F2: return "F2";
    case VKEY_F3: return "F3";
    case VKEY_F4: return "F4";
    case VKEY_F5: return "F5";
    case VKEY_F6: return "F6";
    case VKEY_F7: return "F7";
    case VKEY_F8: return "F8";
    case VKEY_F9: return "F9";
    case VKEY_F10: return "F10";
    case VKEY_F11: return "F11";
    case VKEY_F12: return "F12";
    case VKEY_F13: return "F13";
    case VKEY_F14: return "F14";
    case VKEY_F15: return "F15";
    case VKEY_F16: return "F16";
    case VKEY_F17: return "F17";
    case VKEY_F18: return "F18";
    case VKEY_F19: return "F19";
    case VKEY_F20: return "F20";
    case VKEY_F21: return "F21";
    case VKEY_F22: return "F22";
    case VKEY_F23: return "F23";
    case VKEY_F24: return "F24";
    case VKEY_NUMLOCK: return "NumLock";
    case VKEY_SCROLL: return "ScrollLock";
    case VKEY_LSHIFT: return "Shift";
    case VKEY_RSHIFT: return "Shift";
    case VKEY_LCONTROL: return "Control";
    case VKEY_RCONTROL: return "Control";
    case VKEY_LMENU: return "Alt";
    case VKEY_RMENU: return "Alt";
    case VKEY_BROWSER_BACK: return "BrowserBack";
    case VKEY_BROWSER_FORWARD: return "BrowserForward";
    case VKEY_BROWSER_REFRESH: return "BrowserRefresh";
    case VKEY_BROWSER_STOP: return "BrowserStop";
    case VKEY_BROWSER_SEARCH: return "BrowserSearch";
    case VKEY_BROWSER_FAVORITES: return "BrowserFavorites";
    case VKEY_BROWSER_HOME: return "BrowserHome";
    case VKEY_VOLUME_MUTE: return "AudioVolumeMute";
    case VKEY_VOLUME_DOWN: return "AudioVolumeDown";
    case VKEY_VOLUME_UP: return "AudioVolumeUp";
    case VKEY_MEDIA_NEXT_TRACK: return "MediaTrackNext";
    case VKEY_MEDIA_PREV_TRACK: return "MediaTrackPrevious";
    case VKEY_MEDIA_STOP: return "MediaStop";
    case VKEY_MEDIA_PLAY_PAUSE: return "MediaPlayPause";
    case VKEY_MEDIA_LAUNCH_MAIL: return "LaunchMail";
    case VKEY_MEDIA_LAUNCH_MEDIA_SELECT: return "LaunchMediaPlayer";
    case VKEY_MEDIA_LAUNCH_APP1: return "LaunchApplication1";
    case VKEY_MEDIA_LAUNCH_APP2: return "LaunchApplication2";
    case VKEY_OEM_1: return ";";
    case VKEY_OEM_PLUS: return "=";
    case VKEY_OEM_COMMA: return ",";
    case VKEY_OEM_MINUS: return "-";
    case VKEY_OEM_PERIOD: return ".";
    case VKEY_OEM_2: return "/";
    case VKEY_OEM_3: return "`";
    case VKEY_OEM_4: return "[";
    case VKEY_OEM_5: return "\\";
    case VKEY_OEM_6: return "]";
    case VKEY_OEM_7: return "'";
    case VKEY_OEM_CLEAR: return "Clear";
    default: return "Unknown";
  }
}

KeyboardCode KeyboardCodeFromStr(const std::string& str, bool* shifted) {
  if (str.size() == 1)
    return KeyboardCodeFromCharCode(str[0], shifted);
  else
    return KeyboardCodeFromKeyIdentifier(str, shifted);
}

}  // namespace nu
