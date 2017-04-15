// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_EVENTS_WIN_KEYBOARD_CODES_WIN_H_
#define NATIVEUI_EVENTS_WIN_KEYBOARD_CODES_WIN_H_

namespace nu {

// Windows does not have universal modifier keys, so just use custom defines.
enum KeyboardModifier {
  MASK_SHIFT   = 1 << 1,
  MASK_CONTROL = 1 << 2,
  MASK_ALT     = 1 << 3,
  MASK_META = 1 << 4,
};

enum KeyboardCode {
  VKEY_CANCEL = 0x03,      // VK_CANCEL
  VKEY_BACK = 0x08,        // VK_BACK
  VKEY_TAB = 0x09,         // VK_TAB
  VKEY_CLEAR = 0x0C,       // VK_CLEAR
  VKEY_RETURN = 0x0D,      // VK_RETURN
  VKEY_SHIFT = 0x10,       // VK_SHIFT
  VKEY_CONTROL = 0x11,     // VK_CONTROL
  VKEY_MENU = 0x12,        // VK_MENU
  VKEY_PAUSE = 0x13,       // VK_PAUSE
  VKEY_CAPITAL = 0x14,     // VK_CAPITAL
  VKEY_KANA = 0x15,        // VK_KANA
  VKEY_HANGUL = 0x15,      // VK_HANGUL
  VKEY_JUNJA = 0x17,       // VK_JUNJA
  VKEY_FINAL = 0x18,       // VK_FINAL
  VKEY_HANJA = 0x19,       // VK_HANJA
  VKEY_KANJI = 0x19,       // VK_KANJI
  VKEY_ESCAPE = 0x1B,      // VK_ESCAPE
  VKEY_CONVERT = 0x1C,     // VK_CONVERT
  VKEY_NONCONVERT = 0x1D,  // VK_NONCONVERT
  VKEY_ACCEPT = 0x1E,      // VK_ACCEPT,
  VKEY_MODECHANGE = 0x1F,  // VK_MODECHANGE
  VKEY_SPACE = 0x20,       // VK_SPACE
  VKEY_PRIOR = 0x21,       // VK_PRIOR
  VKEY_NEXT = 0x22,        // VK_NEXT
  VKEY_END = 0x23,         // VK_END
  VKEY_HOME = 0x24,        // VK_HOME
  VKEY_LEFT = 0x25,        // VK_LEFT
  VKEY_UP = 0x26,          // VK_UP
  VKEY_RIGHT = 0x27,       // VK_RIGHT
  VKEY_DOWN = 0x28,        // VK_DOWN
  VKEY_SELECT = 0x29,      // VK_SELECT
  VKEY_PRINT = 0x2A,       // VK_PRINT
  VKEY_EXECUTE = 0x2B,     // VK_EXECUTE
  VKEY_SNAPSHOT = 0x2C,    // VK_SNAPSHOT
  VKEY_INSERT = 0x2D,      // VK_INSERT
  VKEY_DELETE = 0x2E,      // VK_DELETE
  VKEY_HELP = 0x2F,        // VK_HELP
  VKEY_0 = '0',
  VKEY_1 = '1',
  VKEY_2 = '2',
  VKEY_3 = '3',
  VKEY_4 = '4',
  VKEY_5 = '5',
  VKEY_6 = '6',
  VKEY_7 = '7',
  VKEY_8 = '8',
  VKEY_9 = '9',
  VKEY_A = 'A',
  VKEY_B = 'B',
  VKEY_C = 'C',
  VKEY_D = 'D',
  VKEY_E = 'E',
  VKEY_F = 'F',
  VKEY_G = 'G',
  VKEY_H = 'H',
  VKEY_I = 'I',
  VKEY_J = 'J',
  VKEY_K = 'K',
  VKEY_L = 'L',
  VKEY_M = 'M',
  VKEY_N = 'N',
  VKEY_O = 'O',
  VKEY_P = 'P',
  VKEY_Q = 'Q',
  VKEY_R = 'R',
  VKEY_S = 'S',
  VKEY_T = 'T',
  VKEY_U = 'U',
  VKEY_V = 'V',
  VKEY_W = 'W',
  VKEY_X = 'X',
  VKEY_Y = 'Y',
  VKEY_Z = 'Z',
  VKEY_LWIN = 0x5B,       // VK_LWIN
  VKEY_COMMAND = VKEY_LWIN,  // Provide the Mac name for convenience.
  VKEY_RWIN = 0x5C,       // VK_RWIN
  VKEY_APPS = 0x5D,       // VK_APPS
  VKEY_SLEEP = 0x5F,      // VK_SLEEP
  VKEY_NUMPAD0 = 0x60,    // VK_NUMPAD0
  VKEY_NUMPAD1 = 0x61,    // VK_NUMPAD1
  VKEY_NUMPAD2 = 0x62,    // VK_NUMPAD2
  VKEY_NUMPAD3 = 0x63,    // VK_NUMPAD3
  VKEY_NUMPAD4 = 0x64,    // VK_NUMPAD4
  VKEY_NUMPAD5 = 0x65,    // VK_NUMPAD5
  VKEY_NUMPAD6 = 0x66,    // VK_NUMPAD6
  VKEY_NUMPAD7 = 0x67,    // VK_NUMPAD7
  VKEY_NUMPAD8 = 0x68,    // VK_NUMPAD8
  VKEY_NUMPAD9 = 0x69,    // VK_NUMPAD9
  VKEY_MULTIPLY = 0x6A,   // VK_MULTIPLY
  VKEY_ADD = 0x6B,        // VK_ADD
  VKEY_SEPARATOR = 0x6C,  // VK_SEPARATOR
  VKEY_SUBTRACT = 0x6D,   // VK_SUBTRACT
  VKEY_DECIMAL = 0x6E,    // VK_DECIMAL
  VKEY_DIVIDE = 0x6F,     // VK_DIVIDE
  VKEY_F1 = 0x70,   // VK_F1
  VKEY_F2 = 0x71,   // VK_F2
  VKEY_F3 = 0x72,   // VK_F3
  VKEY_F4 = 0x73,   // VK_F4
  VKEY_F5 = 0x74,   // VK_F5
  VKEY_F6 = 0x75,   // VK_F6
  VKEY_F7 = 0x76,   // VK_F7
  VKEY_F8 = 0x77,   // VK_F8
  VKEY_F9 = 0x78,   // VK_F9
  VKEY_F10 = 0x79,  // VK_F10
  VKEY_F11 = 0x7A,  // VK_F11
  VKEY_F12 = 0x7B,  // VK_F12
  VKEY_F13 = 0x7C,  // VK_F13
  VKEY_F14 = 0x7D,  // VK_F14
  VKEY_F15 = 0x7E,  // VK_F15
  VKEY_F16 = 0x7F,  // VK_F16
  VKEY_F17 = 0x80,  // VK_F17
  VKEY_F18 = 0x81,  // VK_F18
  VKEY_F19 = 0x82,  // VK_F19
  VKEY_F20 = 0x83,  // VK_F20
  VKEY_F21 = 0x84,  // VK_F21
  VKEY_F22 = 0x85,  // VK_F22
  VKEY_F23 = 0x86,  // VK_F23
  VKEY_F24 = 0x87,  // VK_F24
  VKEY_NUMLOCK = 0x90,   // VK_NUMLOCK
  VKEY_SCROLL = 0x91,    // VK_SCROLL
  VKEY_LSHIFT = 0xA0,    // VK_LSHIFT,
  VKEY_RSHIFT = 0xA1,    // VK_RSHIFT,
  VKEY_LCONTROL = 0xA2,  // VK_LCONTROL
  VKEY_RCONTROL = 0xA3,  // VK_RCONTROL
  VKEY_LMENU = 0xA4,     // VK_LMENU
  VKEY_RMENU = 0xA5,     // VK_RMENU
  VKEY_BROWSER_BACK = 0xA6,       // VK_BROWSER_BACK
  VKEY_BROWSER_FORWARD = 0xA7,    // VK_BROWSER_FORWARD
  VKEY_BROWSER_REFRESH = 0xA8,    // VK_BROWSER_REFRESH
  VKEY_BROWSER_STOP = 0xA9,       // VK_BROWSER_STOP
  VKEY_BROWSER_SEARCH = 0xAA,     // VK_BROWSER_SEARCH
  VKEY_BROWSER_FAVORITES = 0xAB,  // VK_BROWSER_FAVORITES
  VKEY_BROWSER_HOME = 0xAC,       // VK_BROWSER_HOME
  VKEY_VOLUME_MUTE = 0xAD,        // VK_VOLUME_MUTE
  VKEY_VOLUME_DOWN = 0xAE,        // VK_VOLUME_DOWN
  VKEY_VOLUME_UP = 0xAF,          // VK_VOLUME_UP
  VKEY_MEDIA_NEXT_TRACK = 0xB0,   // VK_MEDIA_NEXT_TRACK
  VKEY_MEDIA_PREV_TRACK = 0xB1,   // VK_MEDIA_PREV_TRACK
  VKEY_MEDIA_STOP = 0xB2,         // VK_MEDIA_STOP
  VKEY_MEDIA_PLAY_PAUSE = 0xB3,   // VK_MEDIA_PLAY_PAUSE
  VKEY_MEDIA_LAUNCH_MAIL = 0xB4,
  VKEY_MEDIA_LAUNCH_MEDIA_SELECT = 0xB5,
  VKEY_MEDIA_LAUNCH_APP1 = 0xB6,
  VKEY_MEDIA_LAUNCH_APP2 = 0xB7,
  VKEY_OEM_1 = 0xBA,         // VK_OEM_1
  VKEY_OEM_PLUS = 0xBB,      // VK_OEM_PLUS
  VKEY_OEM_COMMA = 0xBC,     // VK_OEM_COMMA
  VKEY_OEM_MINUS = 0xBD,     // VK_OEM_MINUS
  VKEY_OEM_PERIOD = 0xBE,    // VK_OEM_PERIOD
  VKEY_OEM_2 = 0xBF,         // VK_OEM_2
  VKEY_OEM_3 = 0xC0,         // VK_OEM_3
  VKEY_OEM_4 = 0xDB,         // VK_OEM_4
  VKEY_OEM_5 = 0xDC,         // VK_OEM_5
  VKEY_OEM_6 = 0xDD,         // VK_OEM_6
  VKEY_OEM_7 = 0xDE,         // VK_OEM_7
  VKEY_OEM_8 = 0xDF,         // VK_OEM_8
  VKEY_OEM_102 = 0xE2,       // VK_OEM_102
  VKEY_PROCESSKEY = 0xE5,    // VK_PROCESSKEY
  VKEY_PACKET = 0xE7,        // VK_PACKET
  VKEY_OEM_ATTN = 0xF0,      // VK_OEM_ATTN
  VKEY_OEM_FINISH = 0xF1,    // VK_OEM_FINISH
  VKEY_OEM_COPY = 0xF2,      // VK_OEM_COPY
  VKEY_DBE_SBCSCHAR = 0xF3,  // VK_DBE_SBCSCHAR
  VKEY_DBE_DBCSCHAR = 0xF4,  // VK_DBE_DBCSCHAR
  VKEY_OEM_BACKTAB = 0xF5,   // VK_OEM_BACKTAB
  VKEY_ATTN = 0xF6,          // VK_ATTN
  VKEY_CRSEL = 0xF7,         // VK_CRSEL
  VKEY_EXSEL = 0xF8,         // VK_EXSEL
  VKEY_EREOF = 0xF9,         // VK_EREOF
  VKEY_PLAY = 0xFA,          // VK_PLAY
  VKEY_ZOOM = 0xFB,          // VK_ZOOM
  VKEY_NONAME = 0xFC,        // VK_NONAME
  VKEY_PA1 = 0xFD,           // VK_PA1
  VKEY_OEM_CLEAR = 0xFE,     // VK_OEM_CLEAR
  VKEY_UNKNOWN = 0,
};

}  // namespace nu

#endif  // NATIVEUI_EVENTS_WIN_KEYBOARD_CODES_WIN_H_
