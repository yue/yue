// Copyright 2017 Cheng Zhao. All rights reserved.
// Copyright 2006 2007 Apple Inc.  All rights reserved.
// Copyright 2006 Michael Emmel mike.emmel@gmail.com
// Copyright 2007 Holger Hans Peter Freyther
// Copyright 2008 Collabora, Ltd.  All rights reserved.
// Copyright 2008 2009 Google Inc.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/events/gtk/keyboard_code_conversion_gtk.h"

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

namespace nu {

KeyboardCode KeyboardCodeFromGdkKeyCode(int keyval) {
  switch (keyval) {
    case GDK_KEY_ISO_Left_Tab:
    case GDK_KEY_3270_BackTab:
    case GDK_KEY_Tab:
      return VKEY_TAB;  // (09) TAB key
    case GDK_KEY_ISO_Enter:
    case GDK_KEY_KP_Enter:
    case GDK_KEY_Return:
      return VKEY_RETURN;  // (0D) Return key
    case GDK_KEY_Shift_L:
    case GDK_KEY_Shift_R:
      return VKEY_SHIFT;  // (10) SHIFT key
    case GDK_KEY_Control_L:
    case GDK_KEY_Control_R:
      return VKEY_CONTROL;  // (11) CTRL key
    case GDK_KEY_Menu:
      return VKEY_APPS;  // (5D) Applications key (Natural keyboard)
    case GDK_KEY_Alt_L:
    case GDK_KEY_Alt_R:
      return VKEY_MENU;  // (12) ALT key
    case GDK_KEY_Kana_Lock:
    case GDK_KEY_Kana_Shift:
      return VKEY_KANA;  // (15) Input Method Editor (IME) Kana mode
    case GDK_KEY_Delete:
    case GDK_KEY_KP_Delete:
      return VKEY_DELETE;  // (2E) DEL key
    case GDK_KEY_0:
    case GDK_KEY_parenright:
      return VKEY_0;  // (30) 0) key
    case GDK_KEY_1:
    case GDK_KEY_exclam:
      return VKEY_1;  // (31) 1 ! key
    case GDK_KEY_2:
    case GDK_KEY_at:
      return VKEY_2;  // (32) 2 & key
    case GDK_KEY_3:
    case GDK_KEY_numbersign:
      return VKEY_3;  // case '3': case '#';
    case GDK_KEY_4:
    case GDK_KEY_dollar:  // (34) 4 key '$';
      return VKEY_4;
    case GDK_KEY_5:
    case GDK_KEY_percent:
      return VKEY_5;  // (35) 5 key  '%'
    case GDK_KEY_6:
    case GDK_KEY_asciicircum:
      return VKEY_6;  // (36) 6 key  '^'
    case GDK_KEY_7:
    case GDK_KEY_ampersand:
      return VKEY_7;  // (37) 7 key  case '&'
    case GDK_KEY_8:
    case GDK_KEY_asterisk:
      return VKEY_8;  // (38) 8 key  '*'
    case GDK_KEY_9:
    case GDK_KEY_parenleft:
      return VKEY_9;  // (39) 9 key '('
    case GDK_KEY_a:
    case GDK_KEY_A:
      return VKEY_A;  // (41) A key case 'a': case 'A': return 0x41;
    case GDK_KEY_b:
    case GDK_KEY_B:
      return VKEY_B;  // (42) B key case 'b': case 'B': return 0x42;
    case GDK_KEY_c:
    case GDK_KEY_C:
      return VKEY_C;  // (43) C key case 'c': case 'C': return 0x43;
    case GDK_KEY_d:
    case GDK_KEY_D:
      return VKEY_D;  // (44) D key case 'd': case 'D': return 0x44;
    case GDK_KEY_e:
    case GDK_KEY_E:
      return VKEY_E;  // (45) E key case 'e': case 'E': return 0x45;
    case GDK_KEY_f:
    case GDK_KEY_F:
      return VKEY_F;  // (46) F key case 'f': case 'F': return 0x46;
    case GDK_KEY_g:
    case GDK_KEY_G:
      return VKEY_G;  // (47) G key case 'g': case 'G': return 0x47;
    case GDK_KEY_h:
    case GDK_KEY_H:
      return VKEY_H;  // (48) H key case 'h': case 'H': return 0x48;
    case GDK_KEY_i:
    case GDK_KEY_I:
      return VKEY_I;  // (49) I key case 'i': case 'I': return 0x49;
    case GDK_KEY_j:
    case GDK_KEY_J:
      return VKEY_J;  // (4A) J key case 'j': case 'J': return 0x4A;
    case GDK_KEY_k:
    case GDK_KEY_K:
      return VKEY_K;  // (4B) K key case 'k': case 'K': return 0x4B;
    case GDK_KEY_l:
    case GDK_KEY_L:
      return VKEY_L;  // (4C) L key case 'l': case 'L': return 0x4C;
    case GDK_KEY_m:
    case GDK_KEY_M:
      return VKEY_M;  // (4D) M key case 'm': case 'M': return 0x4D;
    case GDK_KEY_n:
    case GDK_KEY_N:
      return VKEY_N;  // (4E) N key case 'n': case 'N': return 0x4E;
    case GDK_KEY_o:
    case GDK_KEY_O:
      return VKEY_O;  // (4F) O key case 'o': case 'O': return 0x4F;
    case GDK_KEY_p:
    case GDK_KEY_P:
      return VKEY_P;  // (50) P key case 'p': case 'P': return 0x50;
    case GDK_KEY_q:
    case GDK_KEY_Q:
      return VKEY_Q;  // (51) Q key case 'q': case 'Q': return 0x51;
    case GDK_KEY_r:
    case GDK_KEY_R:
      return VKEY_R;  // (52) R key case 'r': case 'R': return 0x52;
    case GDK_KEY_s:
    case GDK_KEY_S:
      return VKEY_S;  // (53) S key case 's': case 'S': return 0x53;
    case GDK_KEY_t:
    case GDK_KEY_T:
      return VKEY_T;  // (54) T key case 't': case 'T': return 0x54;
    case GDK_KEY_u:
    case GDK_KEY_U:
      return VKEY_U;  // (55) U key case 'u': case 'U': return 0x55;
    case GDK_KEY_v:
    case GDK_KEY_V:
      return VKEY_V;  // (56) V key case 'v': case 'V': return 0x56;
    case GDK_KEY_w:
    case GDK_KEY_W:
      return VKEY_W;  // (57) W key case 'w': case 'W': return 0x57;
    case GDK_KEY_x:
    case GDK_KEY_X:
      return VKEY_X;  // (58) X key case 'x': case 'X': return 0x58;
    case GDK_KEY_y:
    case GDK_KEY_Y:
      return VKEY_Y;  // (59) Y key case 'y': case 'Y': return 0x59;
    case GDK_KEY_z:
    case GDK_KEY_Z:
      return VKEY_Z;  // (5A) Z key case 'z': case 'Z': return 0x5A;
    case GDK_KEY_Meta_L:
    case GDK_KEY_Super_L:
      return VKEY_LWIN;  // (5B) Left Windows key (Microsoft Natural keyboard)
    case GDK_KEY_Meta_R:
    case GDK_KEY_Super_R:
      return VKEY_RWIN;  // (5C) Right Windows key (Natural keyboard)
    case GDK_KEY_semicolon:
    case GDK_KEY_colon:
      return VKEY_OEM_1;  // case ';': case ':': return 0xBA;
      // VKEY_OEM_PLUS (BB) Windows 2000/XP: For any country/region, the '+' key
    case GDK_KEY_plus:
    case GDK_KEY_equal:
      return VKEY_OEM_PLUS;  // case '=': case '+': return 0xBB;
      // VKEY_OEM_COMMA (BC) Windows 2000/XP: For any country/region, the ','
      // key
    case GDK_KEY_comma:
    case GDK_KEY_less:
      return VKEY_OEM_COMMA;  // case ',': case '<': return 0xBC;
      // VKEY_OEM_MINUS (BD) Windows 2000/XP: For any country/region, the '-'
      // key
    case GDK_KEY_minus:
    case GDK_KEY_underscore:
      return VKEY_OEM_MINUS;  // case '-': case '_': return 0xBD;
      // VKEY_OEM_PERIOD (BE) Windows 2000/XP: For any country/region, the '.'
      // key
    case GDK_KEY_period:
    case GDK_KEY_greater:
      return VKEY_OEM_PERIOD;  // case '.': case '>': return 0xBE;
      // VKEY_OEM_2 (BF) Used for miscellaneous characters; it can vary by
      // keyboard. Windows 2000/XP: For the US standard keyboard, the '/?' key
    case GDK_KEY_slash:
    case GDK_KEY_question:
      return VKEY_OEM_2;  // case '/': case '?': return 0xBF;
      // VKEY_OEM_3 (C0) Used for miscellaneous characters; it can vary by
      // keyboard. Windows 2000/XP: For the US standard keyboard, the '`~' key
    case GDK_KEY_asciitilde:
    case GDK_KEY_quoteleft:
      return VKEY_OEM_3;  // case '`': case '~': return 0xC0;
      // VKEY_OEM_4 (DB) Used for miscellaneous characters; it can vary by
      // keyboard. Windows 2000/XP: For the US standard keyboard, the '[{' key
    case GDK_KEY_bracketleft:
    case GDK_KEY_braceleft:
      return VKEY_OEM_4;  // case '[': case '{': return 0xDB;
      // VKEY_OEM_5 (DC) Used for miscellaneous characters; it can vary by
      // keyboard. Windows 2000/XP: For the US standard keyboard, the '\|' key
    case GDK_KEY_backslash:
    case GDK_KEY_bar:
      return VKEY_OEM_5;  // case '\\': case '|': return 0xDC;
      // VKEY_OEM_6 (DD) Used for miscellaneous characters; it can vary by
      // keyboard. Windows 2000/XP: For the US standard keyboard, the ']}' key
    case GDK_KEY_bracketright:
    case GDK_KEY_braceright:
      return VKEY_OEM_6;  // case ']': case '}': return 0xDD;
      // VKEY_OEM_7 (DE) Used for miscellaneous characters; it can vary by
      // keyboard. Windows 2000/XP: For the US standard keyboard, the
      // 'single-quote/double-quote' key
    case GDK_KEY_quoteright:
    case GDK_KEY_quotedbl:
      return VKEY_OEM_7;  // case '\'': case '"': return 0xDE;
    default:
      return static_cast<KeyboardCode>(keyval);
  }
}

}  // namespace nu
