// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_WINDOW_WIN_H_
#define NATIVEUI_WIN_WINDOW_WIN_H_

#include "nativeui/win/focus_manager.h"
#include "nativeui/win/util/win32_window.h"
#include "nativeui/window.h"

namespace nu {

class WindowImpl : public Win32Window {
 public:
  explicit WindowImpl(Window* delegate);

  void SetPixelBounds(const Rect& bounds);
  Rect GetPixelBounds();
  Rect GetContentPixelBounds();

  void SetCapture(ViewImpl* view);
  void ReleaseCapture();

  void SetBackgroundColor(nu::Color color);

  FocusManager* focus_manager() { return &focus_manager_; }
  float scale_factor() const { return scale_factor_; }

 protected:
  CR_BEGIN_MSG_MAP_EX(WindowImpl, Win32Window)
    CR_MSG_WM_CAPTURECHANGED(OnCaptureChanged)
    CR_MSG_WM_CLOSE(OnClose)
    CR_MSG_WM_COMMAND(OnCommand)
    CR_MSG_WM_CTLCOLOREDIT(OnCtlColorStatic)
    CR_MSG_WM_CTLCOLORSTATIC(OnCtlColorStatic)
    CR_MSG_WM_SIZE(OnSize)
    CR_MSG_WM_MOUSEMOVE(OnMouseMove)
    CR_MSG_WM_MOUSELEAVE(OnMouseLeave)
    CR_MSG_WM_MOUSEWHEEL(OnMouseWheel)
    CR_MESSAGE_RANGE_HANDLER_EX(WM_LBUTTONDOWN, WM_MBUTTONDBLCLK, OnMouseClick)
    CR_MESSAGE_RANGE_HANDLER_EX(WM_KEYDOWN, WM_KEYUP, OnKeyEvent)
    CR_MSG_WM_CHAR(OnChar)
    CR_MSG_WM_PAINT(OnPaint)
    CR_MSG_WM_ERASEBKGND(OnEraseBkgnd)
    CR_MESSAGE_HANDLER_EX(WM_DPICHANGED, OnDPIChanged)
  CR_END_MSG_MAP()

 private:
  void OnCaptureChanged(HWND window);
  void OnClose();
  void OnCommand(UINT code, int command, HWND window);
  HBRUSH OnCtlColorStatic(HDC dc, HWND window);
  void OnSize(UINT param, const Size& size);
  void OnMouseMove(UINT flags, const Point& point);
  void OnMouseLeave();
  BOOL OnMouseWheel(bool vertical, UINT flags, int delta, const Point& point);
  LRESULT OnMouseClick(UINT message, WPARAM w_param, LPARAM l_param);
  LRESULT OnKeyEvent(UINT message, WPARAM w_param, LPARAM l_param);
  void OnChar(UINT ch, UINT repeat, UINT flags);
  void OnPaint(HDC dc);
  LRESULT OnEraseBkgnd(HDC dc);
  LRESULT OnDPIChanged(UINT msg, WPARAM w_param, LPARAM l_param);

  void TrackMouse(bool enable);

  FocusManager focus_manager_;
  bool mouse_in_window_ = false;

  // The view that has mouse capture.
  ViewImpl* capture_view_ = nullptr;

  // The background color.
  nu::Color background_color_ = nu::Color(0xFF, 0xFF, 0xFF);

  // The public Window interface.
  Window* delegate_;

  // The scale factor of current window.
  float scale_factor_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_WINDOW_WIN_H_
