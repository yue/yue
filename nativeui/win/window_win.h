// Copyright 2016 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
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
  WindowImpl(const Window::Options& options, Window* delegate);

  void SetPixelBounds(const Rect& bounds);
  Rect GetPixelBounds();
  Rect GetContentPixelBounds();

  void AdvanceFocus();

  void SetCapture(ViewImpl* view);
  void ReleaseCapture();

  bool IsMaximized() const;
  void SetFullscreen(bool fullscreen);
  bool IsFullscreen() const;

  void SetBackgroundColor(nu::Color color);

  void SetWindowStyle(LONG style, bool on);
  bool HasWindowStyle(LONG style) const;
  void ExecuteSystemMenuCommand(int command);

  // Min/max sizes.
  void set_min_size(const Size& min_size) { min_size_ = min_size; }
  Size min_size() const { return min_size_; }
  void set_max_size(const Size& max_size) { max_size_ = max_size; }
  Size max_size() const { return max_size_; }

  Window* delegate() { return delegate_; }
  FocusManager* focus_manager() { return &focus_manager_; }
  ViewImpl* captured_view() const { return captured_view_; }
  float scale_factor() const { return scale_factor_; }

 protected:
  CR_BEGIN_MSG_MAP_EX(WindowImpl, Win32Window)
    // Window events.
    CR_MSG_WM_CAPTURECHANGED(OnCaptureChanged)
    CR_MSG_WM_CLOSE(OnClose)
    CR_MSG_WM_COMMAND(OnCommand)
    CR_MSG_WM_SIZE(OnSize)
    CR_MESSAGE_HANDLER_EX(WM_DPICHANGED, OnDPIChanged)

    // Input events.
    CR_MESSAGE_HANDLER_EX(WM_MOUSEMOVE, OnMouseMove)
    CR_MESSAGE_HANDLER_EX(WM_MOUSELEAVE, OnMouseLeave)
    CR_MSG_WM_MOUSEWHEEL(OnMouseWheel)
    CR_MESSAGE_RANGE_HANDLER_EX(WM_LBUTTONDOWN, WM_MBUTTONDBLCLK, OnMouseClick)
    CR_MESSAGE_RANGE_HANDLER_EX(WM_KEYDOWN, WM_KEYUP, OnKeyEvent)
    CR_MESSAGE_RANGE_HANDLER_EX(WM_SYSKEYDOWN, WM_SYSKEYUP, OnKeyEvent)
    CR_MSG_WM_CHAR(OnChar)

    // Paint events.
    CR_MSG_WM_PAINT(OnPaint)
    CR_MSG_WM_ERASEBKGND(OnEraseBkgnd)
    CR_MSG_WM_CTLCOLOREDIT(OnCtlColorStatic)
    CR_MSG_WM_CTLCOLORSTATIC(OnCtlColorStatic)

    // Non-client area.
    CR_MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
    CR_MESSAGE_HANDLER_EX(WM_NCHITTEST, OnNCHitTest)
    CR_MSG_WM_NCCALCSIZE(OnNCCalcSize)
    CR_MESSAGE_HANDLER_EX(WM_SETCURSOR, OnSetCursor);
  CR_END_MSG_MAP()

 private:
  void OnCaptureChanged(HWND window);
  void OnClose();
  void OnCommand(UINT code, int command, HWND window);
  void OnSize(UINT param, const Size& size);
  LRESULT OnDPIChanged(UINT msg, WPARAM w_param, LPARAM l_param);
  LRESULT OnMouseMove(UINT message, WPARAM w_param, LPARAM l_param);
  LRESULT OnMouseLeave(UINT message, WPARAM w_param, LPARAM l_param);
  BOOL OnMouseWheel(bool vertical, UINT flags, int delta, const Point& point);
  LRESULT OnMouseClick(UINT message, WPARAM w_param, LPARAM l_param);
  LRESULT OnKeyEvent(UINT message, WPARAM w_param, LPARAM l_param);
  void OnChar(UINT ch, UINT repeat, UINT flags);
  void OnPaint(HDC dc);
  LRESULT OnEraseBkgnd(HDC dc);
  HBRUSH OnCtlColorStatic(HDC dc, HWND window);
  void OnGetMinMaxInfo(MINMAXINFO* minmax_info);
  LRESULT OnNCHitTest(UINT msg, WPARAM w_param, LPARAM l_param);
  LRESULT OnNCCalcSize(BOOL mode, LPARAM l_param);
  LRESULT OnSetCursor(UINT message, WPARAM w_param, LPARAM l_param);

  void TrackMouse(bool enable);
  bool GetClientAreaInsets(Insets* insets);
  bool HasSystemFrame() const;

  FocusManager focus_manager_;
  bool mouse_in_window_ = false;

  // True the first time nccalc is called on a sizable widget.
  bool is_first_nccalc_ = true;

  // The view that has mouse capture.
  ViewImpl* captured_view_ = nullptr;

  // Information saved before going into fullscreen mode, used to restore the
  // window afterwards.
  struct SavedWindowInfo {
    LONG style;
    LONG ex_style;
    RECT window_rect;
  } saved_window_info_;
  bool fullscreen_ = false;

  // Min/max size.
  Size min_size_;
  Size max_size_;

  // The background color.
  nu::Color background_color_ = nu::Color(0xFF, 0xFF, 0xFF);

  // The public Window interface.
  Window* delegate_;

  // The scale factor of current window.
  float scale_factor_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_WINDOW_WIN_H_
