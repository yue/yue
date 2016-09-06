// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_WINDOW_WIN_H_
#define NATIVEUI_WIN_WINDOW_WIN_H_

#include "nativeui/win/focus_manager.h"
#include "nativeui/win/window_impl.h"
#include "nativeui/window.h"

namespace nu {

class TopLevelWindow : public WindowImpl {
 public:
  explicit TopLevelWindow(Window* delegate) : delegate_(delegate) {}

  void SetPixelBounds(const Rect& bounds);
  Rect GetPixelBounds();
  Rect GetContentPixelBounds();

  FocusManager* focus_manager() { return &focus_manager_; }

 protected:
  CR_BEGIN_MSG_MAP_EX(TopLevelWindow, WindowImpl)
    CR_MSG_WM_CLOSE(OnClose)
    CR_MSG_WM_COMMAND(OnCommand)
    CR_MSG_WM_SIZE(OnSize)
    CR_MSG_WM_MOUSEMOVE(OnMouseMove)
    CR_MSG_WM_MOUSELEAVE(OnMouseLeave)
    CR_MESSAGE_RANGE_HANDLER_EX(WM_LBUTTONDOWN, WM_MBUTTONDBLCLK, OnMouseClick)
    CR_MSG_WM_CHAR(OnChar)
    CR_MSG_WM_PAINT(OnPaint)
    CR_MSG_WM_ERASEBKGND(OnEraseBkgnd)
  CR_END_MSG_MAP()

 private:
  void OnClose();
  void OnCommand(UINT code, int command, HWND window);
  void OnSize(UINT param, const Size& size);
  void OnMouseMove(UINT flags, const Point& point);
  void OnMouseLeave();
  LRESULT OnMouseClick(UINT message, WPARAM w_param, LPARAM l_param);
  void OnChar(UINT ch, UINT repeat, UINT flags);
  void OnPaint(HDC dc);
  LRESULT OnEraseBkgnd(HDC dc);

  void TrackMouse(bool enable);

  FocusManager focus_manager_;
  bool mouse_in_window_ = false;
  Window* delegate_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_WINDOW_WIN_H_
