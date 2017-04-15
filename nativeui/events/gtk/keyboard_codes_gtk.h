// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_EVENTS_GTK_KEYBOARD_CODES_GTK_H_
#define NATIVEUI_EVENTS_GTK_KEYBOARD_CODES_GTK_H_

namespace nu {

// Code value from gtk/gdk/gdktypes.h.
enum KeyboardModifier {
  MASK_SHIFT   = 1 << 0,
  MASK_CONTROL = 1 << 2,
  MASK_ALT     = 1 << 3,
  MASK_META = 1 << 26,
};

// Code value from gtk/gdk/gdkkeysyms.h.
enum KeyboardCode {
  VKEY_CANCEL = 0xff69,    // GDK_KEY_Cancel
  VKEY_BACK = 0xff08,      // GDK_KEY_BackSpace
  VKEY_TAB = 0xff09,       // GDK_KEY_Tab
  VKEY_CLEAR = 0xff0b,     // GDK_KEY_Clear
  VKEY_RETURN = 0xff0d,    // GDK_KEY_Return
  VKEY_SHIFT = 0xffe1,     // GDK_KEY_Shift_L
  VKEY_CONTROL = 0xffe3,   // GDK_KEY_Control_L
  VKEY_MENU = 0xff67,      // GDK_KEY_Menu
  VKEY_PAUSE = 0xff13,     // GDK_KEY_Pause
  VKEY_CAPITAL = 0xffe5,   // GDK_KEY_Caps_Lock
  VKEY_KANA = 0xff2e,      // GDK_KEY_Kana_Shift
  VKEY_HANGUL = 0xff31,    // GDK_KEY_Hangul
  VKEY_HANJA = 0xff34,     // GDK_KEY_Hangul_Hanja
  VKEY_KANJI = 0xff21,     // GDK_KEY_Kanji
  VKEY_ESCAPE = 0xff1b,    // GDK_KEY_Escape
  VKEY_SPACE = 0x020,      // GDK_KEY_space
  VKEY_PRIOR = 0xff55,     // GDK_KEY_Page_Up
  VKEY_NEXT = 0xff56,      // GDK_KEY_Page_Down
  VKEY_END = 0xff57,       // GDK_KEY_End
  VKEY_HOME = 0xff50,      // GDK_KEY_Home
  VKEY_LEFT = 0xff51,      // GDK_KEY_Left
  VKEY_UP = 0xff52,        // GDK_KEY_Up
  VKEY_RIGHT = 0xff53,     // GDK_KEY_Right
  VKEY_DOWN = 0xff54,      // GDK_KEY_Down
  VKEY_SELECT = 0xff60,    // GDK_KEY_Select
  VKEY_PRINT = 0xff61,     // GDK_KEY_Print
  VKEY_EXECUTE = 0xff62,   // GDK_KEY_Execute
  VKEY_SNAPSHOT = 0xfd1d,  // GDK_KEY_3270_PrintScreen
  VKEY_INSERT = 0xff63,    // GDK_KEY_Insert
  VKEY_DELETE = 0xffff,    // GDK_KEY_Delete
  VKEY_HELP = 0xff6a,      // GDK_KEY_Help
  VKEY_0 = 0x030,  // GDK_KEY_0
  VKEY_1 = 0x031,  // GDK_KEY_1
  VKEY_2 = 0x032,  // GDK_KEY_2
  VKEY_3 = 0x033,  // GDK_KEY_3
  VKEY_4 = 0x034,  // GDK_KEY_4
  VKEY_5 = 0x035,  // GDK_KEY_5
  VKEY_6 = 0x036,  // GDK_KEY_6
  VKEY_7 = 0x037,  // GDK_KEY_7
  VKEY_8 = 0x038,  // GDK_KEY_8
  VKEY_9 = 0x039,  // GDK_KEY_9
  VKEY_A = 0x041,  // GDK_KEY_A
  VKEY_B = 0x042,  // GDK_KEY_B
  VKEY_C = 0x043,  // GDK_KEY_C
  VKEY_D = 0x044,  // GDK_KEY_D
  VKEY_E = 0x045,  // GDK_KEY_E
  VKEY_F = 0x046,  // GDK_KEY_F
  VKEY_G = 0x047,  // GDK_KEY_G
  VKEY_H = 0x048,  // GDK_KEY_H
  VKEY_I = 0x049,  // GDK_KEY_I
  VKEY_J = 0x04a,  // GDK_KEY_J
  VKEY_K = 0x04b,  // GDK_KEY_K
  VKEY_L = 0x04c,  // GDK_KEY_L
  VKEY_M = 0x04d,  // GDK_KEY_M
  VKEY_N = 0x04e,  // GDK_KEY_N
  VKEY_O = 0x04f,  // GDK_KEY_O
  VKEY_P = 0x050,  // GDK_KEY_P
  VKEY_Q = 0x051,  // GDK_KEY_Q
  VKEY_R = 0x052,  // GDK_KEY_R
  VKEY_S = 0x053,  // GDK_KEY_S
  VKEY_T = 0x054,  // GDK_KEY_T
  VKEY_U = 0x055,  // GDK_KEY_U
  VKEY_V = 0x056,  // GDK_KEY_V
  VKEY_W = 0x057,  // GDK_KEY_W
  VKEY_X = 0x058,  // GDK_KEY_X
  VKEY_Y = 0x059,  // GDK_KEY_Y
  VKEY_Z = 0x05a,  // GDK_KEY_Z
  VKEY_LWIN = 0xffe7,  // GDK_KEY_Meta_L
  VKEY_COMMAND = VKEY_LWIN,  // Provide the Mac name for convenience.
  VKEY_RWIN = 0xffe8,  // GDK_KEY_Meta_R
  VKEY_APPS = 0xff67,  // GDK_KEY_Menu
  VKEY_NUMPAD0 = 0xffb0,   // GDK_KEY_KP_0
  VKEY_NUMPAD1 = 0xffb1,   // GDK_KEY_KP_1
  VKEY_NUMPAD2 = 0xffb2,   // GDK_KEY_KP_2
  VKEY_NUMPAD3 = 0xffb3,   // GDK_KEY_KP_3
  VKEY_NUMPAD4 = 0xffb4,   // GDK_KEY_KP_4
  VKEY_NUMPAD5 = 0xffb5,   // GDK_KEY_KP_5
  VKEY_NUMPAD6 = 0xffb6,   // GDK_KEY_KP_6
  VKEY_NUMPAD7 = 0xffb7,   // GDK_KEY_KP_7
  VKEY_NUMPAD8 = 0xffb8,   // GDK_KEY_KP_8
  VKEY_NUMPAD9 = 0xffb9,   // GDK_KEY_KP_9
  VKEY_MULTIPLY = 0xffaa,  // GDK_KEY_KP_Multiply
  VKEY_ADD = 0xffab,       // GDK_KEY_KP_Add
  VKEY_SUBTRACT = 0xffad,  // GDK_KEY_KP_Subtract
  VKEY_DECIMAL = 0xffae,   // GDK_KEY_KP_Decimal
  VKEY_DIVIDE = 0xffaf,    // GDK_KEY_KP_Divide
  VKEY_F1 = 0xffbe,   // GDK_KEY_F1
  VKEY_F2 = 0xffbf,   // GDK_KEY_F2
  VKEY_F3 = 0xffc0,   // GDK_KEY_F3
  VKEY_F4 = 0xffc1,   // GDK_KEY_F4
  VKEY_F5 = 0xffc2,   // GDK_KEY_F5
  VKEY_F6 = 0xffc3,   // GDK_KEY_F6
  VKEY_F7 = 0xffc4,   // GDK_KEY_F7
  VKEY_F8 = 0xffc5,   // GDK_KEY_F8
  VKEY_F9 = 0xffc6,   // GDK_KEY_F9
  VKEY_F10 = 0xffc7,  // GDK_KEY_F10
  VKEY_F11 = 0xffc8,  // GDK_KEY_F11
  VKEY_F12 = 0xffc9,  // GDK_KEY_F12
  VKEY_F13 = 0xffca,  // GDK_KEY_F13
  VKEY_F14 = 0xffcb,  // GDK_KEY_F14
  VKEY_F15 = 0xffcc,  // GDK_KEY_F15
  VKEY_F16 = 0xffcd,  // GDK_KEY_F16
  VKEY_F17 = 0xffce,  // GDK_KEY_F17
  VKEY_F18 = 0xffcf,  // GDK_KEY_F18
  VKEY_F19 = 0xffd0,  // GDK_KEY_F19
  VKEY_F20 = 0xffd1,  // GDK_KEY_F20
  VKEY_F21 = 0xffd2,  // GDK_KEY_F21
  VKEY_F22 = 0xffd3,  // GDK_KEY_F22
  VKEY_F23 = 0xffd4,  // GDK_KEY_F23
  VKEY_F24 = 0xffd5,  // GDK_KEY_F24
  VKEY_NUMLOCK = 0xff7f,   // GDK_KEY_Num_Lock
  VKEY_SCROLL = 0xff14,    // GDK_KEY_Scroll_Lock
  VKEY_LSHIFT = 0xffe1,    // GDK_KEY_Shift_L
  VKEY_RSHIFT = 0xffe2,    // GDK_KEY_Shift_R
  VKEY_LCONTROL = 0xffe3,  // GDK_KEY_Control_L
  VKEY_RCONTROL = 0xffe4,  // GDK_KEY_Control_R
  VKEY_LMENU = 0xff67,     // GDK_KEY_Menu
  VKEY_RMENU = 0xff67,     // GDK_KEY_Menu
  VKEY_BROWSER_BACK = 0x1008ff26,               // GDK_KEY_Back
  VKEY_BROWSER_FORWARD = 0x1008ff27,            // GDK_KEY_Forward
  VKEY_BROWSER_REFRESH = 0x1008ff29,            // GDK_KEY_Refresh
  VKEY_BROWSER_STOP = 0x1008ff28,               // GDK_KEY_Stop
  VKEY_BROWSER_SEARCH = 0x1008ff1b,             // GDK_KEY_Search
  VKEY_BROWSER_FAVORITES = 0x1008ff30,          // GDK_KEY_Favorites
  VKEY_BROWSER_HOME = 0x1008ff18,               // GDK_KEY_HomePage
  VKEY_VOLUME_MUTE = 0x1008ff12,                // GDK_KEY_AudioMute
  VKEY_VOLUME_DOWN = 0x1008ff11,                // GDK_KEY_AudioLowerVolume
  VKEY_VOLUME_UP = 0x1008ff13,                  // GDK_KEY_AudioRaiseVolume
  VKEY_MEDIA_NEXT_TRACK = 0x1008ff17,           // GDK_KEY_AudioNext
  VKEY_MEDIA_PREV_TRACK = 0x1008ff16,           // GDK_KEY_AudioPrev
  VKEY_MEDIA_STOP = 0x1008ff15,                 // GDK_KEY_AudioStop
  VKEY_MEDIA_PLAY_PAUSE = 0x1008ff31,           // GDK_KEY_AudioPause
  VKEY_MEDIA_LAUNCH_MAIL = 0x1008ff19,          // GDK_KEY_Mail
  VKEY_MEDIA_LAUNCH_MEDIA_SELECT = 0x1008ff32,  // GDK_KEY_AudioMedia
  VKEY_MEDIA_LAUNCH_APP1 = 0x1008ff40,          // GDK_KEY_Launch0
  VKEY_MEDIA_LAUNCH_APP2 = 0x1008ff41,          // GDK_KEY_Launch1
  VKEY_OEM_1 = 0x03b,       // GDK_KEY_semicolon
  VKEY_OEM_PLUS = 0x02b,    // GDK_KEY_plus
  VKEY_OEM_COMMA = 0x02c,   // GDK_KEY_comma
  VKEY_OEM_MINUS = 0x02d,   // GDK_KEY_minus
  VKEY_OEM_PERIOD = 0x02e,  // GDK_KEY_period
  VKEY_OEM_2 = 0x02f,       // GDK_KEY_slash
  VKEY_OEM_3 = 0x07e,       // GDK_KEY_asciitilde
  VKEY_OEM_4 = 0x05b,       // GDK_KEY_bracketleft
  VKEY_OEM_5 = 0x05c,       // GDK_KEY_backslash
  VKEY_OEM_6 = 0x05d,       // GDK_KEY_bracketright
  VKEY_OEM_7 = 0x027,       // GDK_KEY_quoteright
  VKEY_OEM_CLEAR = 0xff0b,  // GDK_KEY_Clear
  VKEY_UNKNOWN = 0,
};

}  // namespace nu

#endif  // NATIVEUI_EVENTS_GTK_KEYBOARD_CODES_GTK_H_
