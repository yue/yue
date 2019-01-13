// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_UTIL_SUBWIN_HOLDER_H_
#define NATIVEUI_WIN_UTIL_SUBWIN_HOLDER_H_

#include "nativeui/win/util/win32_window.h"

namespace nu {

// Windows does not allow a child window to created without a parent, so this
// window becomes the temporary parent for the SubwinView childs that are not
// added to any parent yet.
class SubwinHolder : public Win32Window {
 public:
  SubwinHolder();
  ~SubwinHolder() override;

 protected:
  CR_BEGIN_MSG_MAP_EX(SubwinHolder, Win32Window)
    CR_MSG_WM_COMMAND(OnCommand)
    CR_MSG_WM_NOTIFY(OnNotify)
    CR_MSG_WM_CTLCOLOREDIT(OnCtlColorStatic)
    CR_MSG_WM_CTLCOLORSTATIC(OnCtlColorStatic)
    CR_MSG_WM_HSCROLL(OnHScroll)
  CR_END_MSG_MAP()

  // Some controls cache their parents, so after we reparent some controls to
  // a new window, they would still send WM_COMMAND messages to this holder.
  // We need to redirect the messages just like the toplevel window.
  void OnCommand(UINT code, int command, HWND window);
  LRESULT OnNotify(int id, LPNMHDR pnmh);
  HBRUSH OnCtlColorStatic(HDC dc, HWND window);
  void OnHScroll(UINT code, UINT pos, HWND window);
};

}  // namespace nu

#endif  // NATIVEUI_WIN_UTIL_SUBWIN_HOLDER_H_
