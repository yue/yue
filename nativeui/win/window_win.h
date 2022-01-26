// Copyright 2016 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_WINDOW_WIN_H_
#define NATIVEUI_WIN_WINDOW_WIN_H_

#include <set>
#include <vector>

#include "nativeui/win/drag_drop/drag_source.h"
#include "nativeui/win/drag_drop/drop_target.h"
#include "nativeui/win/focus_manager.h"
#include "nativeui/win/util/win32_window.h"
#include "nativeui/window.h"

namespace nu {

class DataObject;

class WindowImpl : public Win32Window,
                   public DragSource::Delegate,
                   public DropTarget::Delegate {
 public:
  WindowImpl(const Window::Options& options, Window* delegate);
  ~WindowImpl() override;

  void SetPixelBounds(const Rect& bounds);
  Rect GetPixelBounds();
  Rect GetContentPixelBounds();

  void AdvanceFocus();

  // Make the window HWND acquire focus without triggering focus events.
  void FocusWithoutEvent();

  bool HandleKeyEvent(const KeyEvent& event);

  void SetCapture(ViewImpl* view);
  void ReleaseCapture();

  bool IsMaximized() const;
  void SetFullscreen(bool fullscreen);
  bool IsFullscreen() const;

  void SetBackgroundColor(nu::Color color);
  void SetHasShadow(bool has);

  int DoDrag(std::vector<Clipboard::Data> data,
             int operations,
             const DragOptions& options);
  void CancelDrag();
  void RegisterDropTarget();

  // Min/max sizes.
  void set_min_size(const Size& min_size) { min_size_ = min_size; }
  Size min_size() const { return min_size_; }
  void set_max_size(const Size& max_size) { max_size_ = max_size; }
  Size max_size() const { return max_size_; }

  FocusManager* focus_manager() { return &focus_manager_; }
  ViewImpl* captured_view() const { return captured_view_; }
  Color background_color() const { return background_color_; }
  bool has_shadow() const { return has_shadow_; }
  bool drag_drop_in_progress() const { return drag_drop_in_progress_; }
  float scale_factor() const { return scale_factor_; }
  Window* delegate() { return delegate_; }

 protected:
  CR_BEGIN_MSG_MAP_EX(WindowImpl, Win32Window)
    // Window events.
    CR_MSG_WM_CAPTURECHANGED(OnCaptureChanged)
    CR_MSG_WM_CLOSE(OnClose)
    CR_MSG_WM_COMMAND(OnCommand)
    CR_MSG_WM_ENTERMENULOOP(OnMenuShow)
    CR_MSG_WM_EXITMENULOOP(OnMenuHide)
    CR_MSG_WM_NOTIFY(OnNotify)
    CR_MSG_WM_SIZE(OnSize)
    CR_MSG_WM_SETFOCUS(OnFocus)
    CR_MSG_WM_KILLFOCUS(OnBlur)
    CR_MESSAGE_HANDLER_EX(WM_DPICHANGED, OnDPIChanged)

    // Input events.
    CR_MESSAGE_HANDLER_EX(WM_MOUSEMOVE, OnMouseMove)
    CR_MESSAGE_HANDLER_EX(WM_MOUSELEAVE, OnMouseLeave)
    CR_MESSAGE_RANGE_HANDLER_EX(WM_MOUSEWHEEL, WM_MOUSEHWHEEL, OnMouseWheel)
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
    CR_MESSAGE_HANDLER_EX(WM_SETCURSOR, OnSetCursor)
  CR_END_MSG_MAP()

 private:
  void OnCaptureChanged(HWND window);
  void OnClose();
  void OnCommand(UINT code, int command, HWND window);
  void OnMenuShow(BOOL is_popup);
  void OnMenuHide(BOOL is_popup);
  LRESULT OnNotify(int id, LPNMHDR pnmh);
  void OnSize(UINT param, const Size& size);
  void OnFocus(HWND old);
  void OnBlur(HWND old);
  LRESULT OnDPIChanged(UINT msg, WPARAM w_param, LPARAM l_param);
  LRESULT OnMouseMove(UINT message, WPARAM w_param, LPARAM l_param);
  LRESULT OnMouseLeave(UINT message, WPARAM w_param, LPARAM l_param);
  LRESULT OnMouseWheel(UINT message, WPARAM w_param, LPARAM l_param);
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

  // DragSource::Delegate:
  void OnDragSourceCancel() override;
  void OnDragSourceDrop() override;
  void OnDragSourceMove() override;

  // DropTarget::Delegate:
  int OnDragEnter(IDataObject* data, int effect, const Point& point) override;
  int OnDragOver(IDataObject* data, int effect, const Point& point) override;
  void OnDragLeave(IDataObject* data) override;
  int OnDrop(IDataObject* data, int effect, const Point& point) override;

  void TrackMouse(bool enable);
  bool GetClientAreaInsets(Insets* insets);

  FocusManager focus_manager_;
  bool mouse_in_window_ = false;

  // Ignore focus event.
  bool ignore_focus_ = false;

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

  // Whether there is native shadow.
  bool has_shadow_ = true;

  // Drag and drop.
  Microsoft::WRL::ComPtr<DragSource> drag_source_;
  scoped_refptr<DataObject> drag_data_;
  scoped_refptr<DropTarget> drop_target_;

  bool drag_drop_in_progress_ = false;

  // The scale factor of current window.
  float scale_factor_;

  // The public Window interface.
  Window* delegate_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_WINDOW_WIN_H_
