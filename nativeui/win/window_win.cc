// Copyright 2016 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE.chromium file.

#include "nativeui/win/window_win.h"

#include <dwmapi.h>
#include <shellapi.h>

#include <limits>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "base/strings/utf_string_conversions.h"
#include "base/win/windows_version.h"
#include "nativeui/accelerator.h"
#include "nativeui/accelerator_manager.h"
#include "nativeui/events/event.h"
#include "nativeui/events/win/event_win.h"
#include "nativeui/gfx/geometry/insets.h"
#include "nativeui/gfx/geometry/rect_conversions.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/win/double_buffer.h"
#include "nativeui/gfx/win/native_theme.h"
#include "nativeui/gfx/win/painter_win.h"
#include "nativeui/menu_bar.h"
#include "nativeui/state.h"
#include "nativeui/win/drag_drop/clipboard_util.h"
#include "nativeui/win/drag_drop/data_object.h"
#include "nativeui/win/menu_base_win.h"
#include "nativeui/win/screen_win.h"
#include "nativeui/win/subwin_view.h"
#include "nativeui/win/util/hwnd_util.h"
#include "third_party/yoga/Yoga.h"

namespace nu {

namespace {

// Default window style for frameless window.
const DWORD kWindowDefaultFramelessStyle =
    WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

// The thickness of an auto-hide taskbar in pixels.
const int kAutoHideTaskbarThicknessPx = 2;

// Compute extends window style from options.
inline DWORD ComputeWindowExStyle(const Window::Options& options) {
  DWORD style = 0;
  if (options.transparent)
    style |= WS_EX_LAYERED;
  if (options.no_activate)
    style |= WS_EX_NOACTIVATE;
  return style;
}

// Convert between window and client areas.
Size ContentToWindowSize(Win32Window* window, bool has_menu_bar,
                         const Size& size) {
  RECT rect = Rect(size).ToRECT();
  AdjustWindowRectEx(&rect, window->window_style(), has_menu_bar,
                     window->window_ex_style());
  return Rect(rect).size();
}

SizeF WindowToContentSize(Win32Window* window, bool has_menu_bar,
                          const SizeF& size) {
  RECT rect = { 0 };
  AdjustWindowRectEx(&rect, window->window_style(), has_menu_bar,
                     window->window_ex_style());
  Rect p(rect);
  return SizeF(size.width() - p.width(), size.height() - p.height());
}

inline bool IsShiftPressed() {
  return (::GetKeyState(VK_SHIFT) & 0x8000) == 0x8000;
}

}  // namespace

WindowImpl::WindowImpl(const Window::Options& options, Window* delegate)
    : Win32Window(L"", NULL,
                  options.frame ? kWindowDefaultStyle
                                : kWindowDefaultFramelessStyle,
                  ComputeWindowExStyle(options)),
      scale_factor_(GetScaleFactorForHWND(hwnd())),
      delegate_(delegate) {
  if (options.frame) {
    // Normal window always has shadow.
    has_shadow_ = true;
  } else {
    // Show shadow for frameless window by default.
    SetHasShadow(true);
    // First nccalcszie (during CreateWindow) for captioned windows is
    // deliberately ignored so force a second one here to get the right
    // non-client set up.
    ::SetWindowPos(hwnd(), NULL, 0, 0, 0, 0,
                   SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE |
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
  }
  if (options.transparent) {
    // Change default background color to transparent.
    background_color_ = Color(0, 0, 0, 0);
  }
  NativeTheme* theme = State::GetCurrent()->GetNativeTheme();
  if (theme->IsAppDarkMode()) {
    theme->EnableDarkModeForWindow(hwnd());
  }
}

WindowImpl::~WindowImpl() {
  if (drag_drop_in_progress_)
    CancelDrag();
}

void WindowImpl::SetPixelBounds(const Rect& bounds) {
  SetWindowPos(hwnd(), NULL,
               bounds.x(), bounds.y(), bounds.width(), bounds.height(),
               SWP_NOACTIVATE | SWP_NOZORDER);
}

Rect WindowImpl::GetPixelBounds() {
  RECT r;
  GetWindowRect(hwnd(), &r);
  return Rect(r);
}

Rect WindowImpl::GetContentPixelBounds() {
  RECT r;
  GetClientRect(hwnd(), &r);
  POINT point = { r.left, r.top };
  ClientToScreen(hwnd(), &point);
  return Rect(point.x, point.y, r.right - r.left, r.bottom - r.top);
}

void WindowImpl::AdvanceFocus() {
  focus_manager_.set_show_focus_ring(true);
  focus_manager_.AdvanceFocus(delegate_->GetContentView()->GetNative(),
                              IsShiftPressed());
}

void WindowImpl::FocusWithoutEvent() {
  ignore_focus_ = true;
  ::SetFocus(hwnd());
  ignore_focus_ = false;
}

bool WindowImpl::HandleKeyEvent(const KeyEvent& event) {
  if (event.type == EventType::KeyDown && delegate_->GetMenuBar()) {
    Accelerator accelerator(event);
    int id = delegate_->GetMenuBar()->accel_manager()->Process(accelerator);
    if (id != -1) {
      DispatchCommandToItem(delegate_->GetMenuBar(), id);
      return true;
    }
  }
  return false;
}

void WindowImpl::SetCapture(ViewImpl* view) {
  captured_view_ = view;
  ::SetCapture(hwnd());
}

void WindowImpl::ReleaseCapture() {
  if (::GetCapture() == hwnd())
    ::ReleaseCapture();
}

bool WindowImpl::IsMaximized() const {
  return !!::IsZoomed(hwnd()) && !IsFullscreen();
}

void WindowImpl::SetFullscreen(bool fullscreen) {
  // Save current window state if not already fullscreen.
  if (!fullscreen_) {
    saved_window_info_.style = ::GetWindowLong(hwnd(), GWL_STYLE);
    saved_window_info_.ex_style = ::GetWindowLong(hwnd(), GWL_EXSTYLE);
    ::GetWindowRect(hwnd(), &saved_window_info_.window_rect);
  }

  fullscreen_ = fullscreen;

  if (fullscreen_) {
    // Set new window style and size.
    ::SetWindowLong(hwnd(), GWL_STYLE,
                    saved_window_info_.style & ~(WS_CAPTION | WS_THICKFRAME));
    ::SetWindowLong(hwnd(), GWL_EXSTYLE,
                    saved_window_info_.ex_style & ~(WS_EX_DLGMODALFRAME |
                    WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

    // On expand, if we're given a window_rect, grow to it, otherwise do
    // not resize.
    MONITORINFO monitor_info;
    monitor_info.cbSize = sizeof(monitor_info);
    ::GetMonitorInfo(::MonitorFromWindow(hwnd(), MONITOR_DEFAULTTONEAREST),
                     &monitor_info);
    Rect window_rect(monitor_info.rcMonitor);
    ::SetWindowPos(hwnd(), NULL, window_rect.x(), window_rect.y(),
                   window_rect.width(), window_rect.height(),
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
  } else {
    // Reset original window style and size.  The multiple window size/moves
    // here are ugly, but if SetWindowPos() doesn't redraw, the taskbar won't be
    // repainted.  Better-looking methods welcome.
    ::SetWindowLong(hwnd(), GWL_STYLE, saved_window_info_.style);
    ::SetWindowLong(hwnd(), GWL_EXSTYLE, saved_window_info_.ex_style);

    // On restore, resize to the previous saved rect size.
    Rect new_rect(saved_window_info_.window_rect);
    ::SetWindowPos(hwnd(), NULL, new_rect.x(), new_rect.y(), new_rect.width(),
                   new_rect.height(),
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
  }
}

bool WindowImpl::IsFullscreen() const {
  return fullscreen_;
}

void WindowImpl::SetBackgroundColor(nu::Color color) {
  background_color_ = color;
  ::InvalidateRect(hwnd(), NULL, TRUE);
}

void WindowImpl::SetHasShadow(bool has) {
  if (delegate_->HasFrame())
    return;
  BOOL enabled = FALSE;
  if (!SUCCEEDED(DwmIsCompositionEnabled(&enabled)) || !enabled)
    return;
  has_shadow_ = has;
  MARGINS shadow = {0};
  if (has)
    shadow = {0, 0, 1, 0};
  ::DwmExtendFrameIntoClientArea(hwnd(), &shadow);
}

int WindowImpl::DoDrag(std::vector<Clipboard::Data> data,
                       int operations,
                       const DragOptions& options) {
  if (drag_source_)
    return DRAG_OPERATION_NONE;

  drag_source_ = DragSource::Create(this);
  drag_data_ = new DataObject(std::move(data));
  drag_drop_in_progress_ = true;

  if (options.image) {
    Microsoft::WRL::ComPtr<IDragSourceHelper> helper;
    if (SUCCEEDED(CoCreateInstance(CLSID_DragDropHelper, 0,
                                   CLSCTX_INPROC_SERVER,
                                   IID_PPV_ARGS(&helper)))) {
      auto size = options.image->GetSize();
      // InitializeFromBitmap() takes ownership of |hbitmap|.
      SHDRAGIMAGE sdi = {{0}};
      sdi.sizeDragImage.cx = size.width();
      sdi.sizeDragImage.cy = size.height();
      sdi.crColorKey = 0x00FFFFFF;
      sdi.hbmpDragImage = GetBitmapFromImage(options.image.get());
      helper->InitializeFromBitmap(&sdi, drag_data_.get());
    }
  }

  DWORD effect;
  HRESULT result = DoDragDrop(
      drag_data_.get(), drag_source_.Get(), operations, &effect);

  drag_drop_in_progress_ = false;
  drag_source_ = nullptr;
  drag_data_ = nullptr;

  if (result != DRAGDROP_S_DROP)
    effect = DRAG_OPERATION_NONE;

  return effect;
}

void WindowImpl::CancelDrag() {
  if (drag_source_)
    drag_source_->CancelDrag();
  drag_drop_in_progress_ = false;
  drag_source_ = nullptr;
  drag_data_ = nullptr;
}

void WindowImpl::RegisterDropTarget() {
  if (!drop_target_)
    drop_target_ = new DropTarget(hwnd(), this);
}

void WindowImpl::OnCaptureChanged(HWND window) {
  if (captured_view_) {
    captured_view_->OnCaptureLost();
    captured_view_ = nullptr;
  }
}

void WindowImpl::OnClose() {
  if (!delegate_->should_close ||
      delegate_->should_close(delegate_)) {
    delegate_->NotifyWindowClosed();
    SetMsgHandled(false);
  }
}

void WindowImpl::OnCommand(UINT code, int command, HWND window) {
  if (!code && !window && delegate_->GetMenuBar()) {
    DispatchCommandToItem(delegate_->GetMenuBar(), command);
    return;
  } else if (::GetParent(window) != hwnd()) {
    LOG(ERROR) << "Received notification " << code << " " << command
               << " from a non-child window";
    return;
  }

  auto* control = reinterpret_cast<SubwinView*>(GetWindowUserData(window));
  control->OnCommand(code, command);
}

void WindowImpl::OnMenuShow(BOOL is_popup) {
  if (!is_popup && delegate_->GetMenuBar())
    delegate_->GetMenuBar()->OnMenuShow();
}

void WindowImpl::OnMenuHide(BOOL is_popup) {
  if (!is_popup && delegate_->GetMenuBar())
    delegate_->GetMenuBar()->OnMenuHide();
}

LRESULT WindowImpl::OnNotify(int id, LPNMHDR pnmh) {
  HWND window = pnmh->hwndFrom;
  if (::GetParent(window) != hwnd())
    return 0;
  auto* control = reinterpret_cast<SubwinView*>(GetWindowUserData(window));
  return control->OnNotify(id, pnmh);
}

void WindowImpl::OnSize(UINT param, const Size& size) {
  if (!delegate_->GetContentView())
    return;
  delegate_->GetContentView()->GetNative()->SizeAllocate(Rect(size));
  RedrawWindow(hwnd(), NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void WindowImpl::OnFocus(HWND old) {
  if (ignore_focus_)
    return;

  // Refocusing a window wouldn't bring focus back to child window, we have
  // to manually focus to recover focus state.
  ViewImpl* focused_view = focus_manager()->focused_view();
  if (focused_view && focused_view->type() == ControlType::Subwin)
    focused_view->SetFocus(true);

  delegate_->on_focus.Emit(delegate_);
  SetMsgHandled(false);
}

void WindowImpl::OnBlur(HWND old) {
  delegate_->on_blur.Emit(delegate_);
  SetMsgHandled(false);
}

LRESULT WindowImpl::OnDPIChanged(UINT msg, WPARAM w_param, LPARAM l_param) {
  float new_scale_factor = GetScalingFactorFromDPI(LOWORD(w_param));
  if (new_scale_factor != scale_factor_) {
    scale_factor_ = new_scale_factor;
    // Notify the content view of DPI change.
    delegate_->GetContentView()->GetNative()->BecomeContentView(this);
    // Move to the new window position under new DPI.
    SetPixelBounds(Rect(*reinterpret_cast<RECT*>(l_param)));
  }
  return 1;
}

LRESULT WindowImpl::OnMouseMove(UINT message, WPARAM w_param, LPARAM l_param) {
  Win32Message msg = {message, w_param, l_param};
  if (!mouse_in_window_) {
    mouse_in_window_ = true;
    if (!captured_view_)
      delegate_->GetContentView()->GetNative()->OnMouseEnter(&msg);
    TrackMouse(true);
  }

  if (captured_view_) {
    captured_view_->OnMouseMove(&msg);
    return 0;
  }

  delegate_->GetContentView()->GetNative()->OnMouseMove(&msg);
  return 0;
}

LRESULT WindowImpl::OnMouseLeave(UINT message, WPARAM w_param, LPARAM l_param) {
  TrackMouse(false);
  mouse_in_window_ = false;

  Win32Message msg = {message, w_param, l_param};
  if (captured_view_) {
    captured_view_->OnMouseLeave(&msg);
    return 0;
  }

  delegate_->GetContentView()->GetNative()->OnMouseLeave(&msg);
  return 0;
}

LRESULT WindowImpl::OnMouseWheel(UINT message, WPARAM w_param, LPARAM l_param) {
  // WM_MOUSEWHEEL uses screen coord, but in View we use client coord.
  POINT p = { CR_GET_X_LPARAM(l_param), CR_GET_Y_LPARAM(l_param) };
  ::ScreenToClient(hwnd(), &p);
  Win32Message msg = {message, w_param, MAKELPARAM(p.x, p.y)};
  if (!delegate_->GetContentView()->GetNative()->OnMouseWheel(&msg))
    SetMsgHandled(false);
  return 0;
}

LRESULT WindowImpl::OnMouseClick(UINT message, WPARAM w_param, LPARAM l_param) {
  Win32Message msg = {message, w_param, l_param};
  if (captured_view_) {
    captured_view_->OnMouseClick(&msg);
    return 0;
  }

  // Pass the event to view.
  if (!delegate_->GetContentView()->GetNative()->OnMouseClick(&msg))
    SetMsgHandled(false);
  return 0;
}

LRESULT WindowImpl::OnKeyEvent(UINT message, WPARAM w_param, LPARAM l_param) {
  // First pass the event to view.
  Win32Message msg = {message, w_param, l_param};
  if (focus_manager()->focused_view() &&
      focus_manager()->focused_view()->OnKeyEvent(&msg))
    return 0;

  // If no one handles it then pass the event to menu.
  if (!HandleKeyEvent(KeyEvent(&msg, delegate_->GetContentView()->GetNative())))
    SetMsgHandled(false);
  return 0;
}

void WindowImpl::OnChar(UINT ch, UINT repeat, UINT flags) {
  if (ch == VK_TAB) {
    if (!focus_manager()->show_focus_ring() &&
        focus_manager()->focused_view()) {
      // When pressing TAB on window for the first time, and there is a focused
      // view, make the focus ring show on the view.
      focus_manager()->set_show_focus_ring(true);
      focus_manager()->focused_view()->Invalidate();
    } else {
      // Otherwise advance focus to next view.
      AdvanceFocus();
    }
  }
}

void WindowImpl::OnPaint(HDC) {
  PAINTSTRUCT ps;
  BeginPaint(hwnd(), &ps);

  // Always update the whole buffer for transparent window.
  Rect bounds(GetContentPixelBounds());
  Rect dirty;
  if (delegate_->IsTransparent())
    dirty = Rect(bounds.size());
  else
    dirty = Rect(ps.rcPaint);

  // Window may be resized to no content.
  if (dirty.IsEmpty())
    return;

  base::win::ScopedGetDC dc(hwnd());
  {
    // Double buffering the drawing.
    DoubleBuffer buffer(dc, bounds.size(), dirty, dirty.origin());

    // Draw.
    {
      // Background.
      PainterWin painter(buffer.dc(), bounds.size(), scale_factor_);
      painter.SetColor(background_color_);
      painter.FillRectPixel(dirty);

      // Controls.
      delegate_->GetContentView()->GetNative()->Draw(&painter, dirty);
    }

    // Update layered window.
    if (delegate_->IsTransparent()) {
      RECT wr;
      ::GetWindowRect(hwnd(), &wr);
      SIZE size = {wr.right - wr.left, wr.bottom - wr.top};
      POINT position = {wr.left, wr.top};
      POINT zero = {0, 0};
      BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};

      buffer.SetNoCopy();
      ::UpdateLayeredWindow(hwnd(), NULL, &position, &size, buffer.dc(),
                            &zero, 0, &blend, ULW_ALPHA);
    }
  }

  EndPaint(hwnd(), &ps);
}

LRESULT WindowImpl::OnEraseBkgnd(HDC dc) {
  // Needed to prevent resize flicker.
  return 1;
}

HBRUSH WindowImpl::OnCtlColorStatic(HDC dc, HWND window) {
  auto* control = reinterpret_cast<SubwinView*>(GetWindowUserData(window));
  HBRUSH brush = NULL;
  SetMsgHandled(control->OnCtlColor(dc, &brush));
  return brush;
}

void WindowImpl::OnGetMinMaxInfo(MINMAXINFO* minmax_info) {
  minmax_info->ptMinTrackSize.x = min_size_.width();
  minmax_info->ptMinTrackSize.y = min_size_.height();
  if (max_size_.IsEmpty()) {
    LONG v = std::numeric_limits<LONG>::max();
    minmax_info->ptMaxTrackSize.x = v;
    minmax_info->ptMaxTrackSize.y = v;
  } else {
    minmax_info->ptMaxTrackSize.x = max_size_.width();
    minmax_info->ptMaxTrackSize.y = max_size_.height();
  }
  SetMsgHandled(false);
}

LRESULT WindowImpl::OnNCHitTest(UINT msg, WPARAM w_param, LPARAM l_param) {
  // Only override this for frameless window.
  if (delegate_->HasFrame()) {
    SetMsgHandled(false);
    return 0;
  }

  // Do nothing for non-client area for now.
  LRESULT hit = ::DefWindowProc(hwnd(), msg, w_param, l_param);
  if (hit != HTCLIENT)
    return hit;

  // Convert point to client area.
  POINT temp = { CR_GET_X_LPARAM(l_param), CR_GET_Y_LPARAM(l_param) };
  ::MapWindowPoints(HWND_DESKTOP, hwnd(), &temp, 1);
  Point point(temp);

  // Calculate the resize handle.
  if (delegate_->IsResizable() && !(IsMaximized() || IsFullscreen())) {
    Rect bounds = GetPixelBounds();
    int border_thickness = GetFrameThickness(scale_factor());
    int corner_width =
        border_thickness * 2 +
        GetSystemMetricsForScaleFactor(scale_factor(), SM_CXBORDER);

    if (point.x() < border_thickness) {
      if (point.y() < corner_width)
        return HTTOPLEFT;
      else if (point.y() >= (bounds.height() - border_thickness))
        return HTBOTTOMLEFT;
      else
        return HTLEFT;
    } else if (point.x() >= (bounds.width() - border_thickness)) {
      if (point.y() < corner_width)
        return HTTOPRIGHT;
      else if (point.y() >= (bounds.height() - border_thickness))
        return HTBOTTOMRIGHT;
      else
        return HTRIGHT;
    } else if (point.y() < border_thickness) {
      if (point.x() < corner_width)
        return HTTOPLEFT;
      else if (point.x() >= (bounds.width() - corner_width))
        return HTTOPRIGHT;
      else
        return HTTOP;
    } else if (point.y() >= (bounds.height() - border_thickness)) {
      if (point.x() < corner_width)
        return HTBOTTOMLEFT;
      else if (point.x() >= (bounds.width() - corner_width))
        return HTBOTTOMRIGHT;
      else
        return HTBOTTOM;
    }
  }

  // Get result from content view.
  return delegate_->GetContentView()->GetNative()->HitTest(point);
}

LRESULT WindowImpl::OnNCCalcSize(BOOL mode, LPARAM l_param) {
  // We only override the default handling if we need to specify a custom
  // non-client edge width. Note that in most cases "no insets" means no
  // custom width, but in fullscreen mode or when the NonClientFrameView
  // requests it, we want a custom width of 0.

  // Let User32 handle the first nccalcsize for captioned windows
  // so it updates its internal structures (specifically caption-present)
  // Without this Tile & Cascade windows won't work.
  // See http://code.google.com/p/chromium/issues/detail?id=900
  if (is_first_nccalc_) {
    is_first_nccalc_ = false;
    if (HasWindowStyle(WS_CAPTION)) {
      SetMsgHandled(false);
      return 0;
    }
  }

  RECT* client_rect = mode ?
      &(reinterpret_cast<NCCALCSIZE_PARAMS*>(l_param)->rgrc[0]) :
      reinterpret_cast<RECT*>(l_param);

  HMONITOR monitor = MonitorFromWindow(hwnd(), MONITOR_DEFAULTTONULL);
  if (!monitor) {
    // We might end up here if the window was previously minimized and the
    // user clicks on the taskbar button to restore it in the previous
    // position. In that case WM_NCCALCSIZE is sent before the window
    // coordinates are restored to their previous values, so our (left,top)
    // would probably be (-32000,-32000) like all minimized windows. So the
    // above MonitorFromWindow call fails, but if we check the window rect
    // given with WM_NCCALCSIZE (which is our previous restored window
    // position) we will get the correct monitor handle.
    monitor = MonitorFromRect(client_rect, MONITOR_DEFAULTTONULL);
    if (!monitor) {
      // This is probably an extreme case that we won't hit, but if we don't
      // intersect any monitor, let us not adjust the client rect since our
      // window will not be visible anyway.
      return 0;
    }
  }

  Insets insets;
  bool got_insets = GetClientAreaInsets(&insets);
  if (!got_insets && !IsFullscreen() && !(mode && !delegate_->HasFrame())) {
    SetMsgHandled(FALSE);
    return 0;
  }

  client_rect->left += insets.left();
  client_rect->top += insets.top();
  client_rect->bottom -= insets.bottom();
  client_rect->right -= insets.right();
  if (IsMaximized()) {
    // Find all auto-hide taskbars along the screen edges and adjust in by the
    // thickness of the auto-hide taskbar on each such edge, so the window isn't
    // treated as a "fullscreen app", which would cause the taskbars to
    // disappear.
    if (MonitorHasAutohideTaskbarForEdge(ABE_LEFT, monitor))
      client_rect->left += kAutoHideTaskbarThicknessPx;
    if (MonitorHasAutohideTaskbarForEdge(ABE_TOP, monitor)) {
      if (delegate_->HasFrame()) {
        // Tricky bit.  Due to a bug in DwmDefWindowProc()'s handling of
        // WM_NCHITTEST, having any nonclient area atop the window causes the
        // caption buttons to draw onscreen but not respond to mouse
        // hover/clicks.
        // So for a taskbar at the screen top, we can't push the
        // client_rect->top down; instead, we move the bottom up by one pixel,
        // which is the smallest change we can make and still get a client area
        // less than the screen size. This is visibly ugly, but there seems to
        // be no better solution.
        --client_rect->bottom;
      } else {
        client_rect->top += kAutoHideTaskbarThicknessPx;
      }
    }
    if (MonitorHasAutohideTaskbarForEdge(ABE_RIGHT, monitor))
      client_rect->right -= kAutoHideTaskbarThicknessPx;
    if (MonitorHasAutohideTaskbarForEdge(ABE_BOTTOM, monitor))
      client_rect->bottom -= kAutoHideTaskbarThicknessPx;

    // We cannot return WVR_REDRAW when there is nonclient area, or Windows
    // exhibits bugs where client pixels and child HWNDs are mispositioned by
    // the width/height of the upper-left nonclient area.
    return 0;
  }

  // If the window bounds change, we're going to relayout and repaint anyway.
  // Returning WVR_REDRAW avoids an extra paint before that of the old client
  // pixels in the (now wrong) location, and thus makes actions like resizing a
  // window from the left edge look slightly less broken.
  // We special case when left or top insets are 0, since these conditions
  // actually require another repaint to correct the layout after glass gets
  // turned on and off.
  if (insets.left() == 0 || insets.top() == 0)
    return 0;
  return mode ? WVR_REDRAW : 0;
}

LRESULT WindowImpl::OnSetCursor(UINT message, WPARAM w_param, LPARAM l_param) {
  // Do not override cursor of child windows.
  if (reinterpret_cast<HWND>(w_param) != hwnd()) {
    SetMsgHandled(false);
    return FALSE;
  }

  // Ask the views for cursor in client area.
  if (LOWORD(l_param) == HTCLIENT) {
    // The l_param does not include mouse position.
    POINT p;
    ::GetCursorPos(&p);
    ::ScreenToClient(hwnd(), &p);
    Win32Message msg = {message, w_param, MAKELPARAM(p.x, p.y)};

    // Set view cursors.
    ViewImpl* view = captured_view_ ? captured_view_
                                    : delegate_->GetContentView()->GetNative();
    if (view->OnSetCursor(&msg)) {
      return TRUE;
    } else {
      SetMsgHandled(false);
      return FALSE;
    }
  }

  // Reimplement the necessary default behavior here. Calling DefWindowProc can
  // trigger weird non-client painting for non-glass windows with custom frames.
  // Using a ScopedRedrawLock to prevent caption rendering artifacts may allow
  // content behind this window to incorrectly paint in front of this window.
  // Invalidating the window to paint over either set of artifacts is not ideal.
  wchar_t* cursor = IDC_ARROW;
  switch (LOWORD(l_param)) {
    case HTSIZE:
      cursor = IDC_SIZENWSE;
      break;
    case HTLEFT:
    case HTRIGHT:
      cursor = IDC_SIZEWE;
      break;
    case HTTOP:
    case HTBOTTOM:
      cursor = IDC_SIZENS;
      break;
    case HTTOPLEFT:
    case HTBOTTOMRIGHT:
      cursor = IDC_SIZENWSE;
      break;
    case HTTOPRIGHT:
    case HTBOTTOMLEFT:
      cursor = IDC_SIZENESW;
      break;
    case LOWORD(HTERROR):  // Use HTERROR's LOWORD value for valid comparison.
      SetMsgHandled(false);
      break;
    default:
      // Use the default value, IDC_ARROW.
      break;
  }
  ::SetCursor(::LoadCursor(NULL, cursor));
  return TRUE;
}

void WindowImpl::OnDragSourceCancel() {
}

void WindowImpl::OnDragSourceDrop() {
  // data->set_in_drag_loop(false);
}

void WindowImpl::OnDragSourceMove() {
}

int WindowImpl::OnDragEnter(IDataObject* data, int effect, const Point& point) {
  return delegate_->GetContentView()->GetNative()->OnDragEnter(
      data, effect, point);
}

int WindowImpl::OnDragOver(IDataObject* data, int effect, const Point& point) {
  return delegate_->GetContentView()->GetNative()->OnDragUpdate(
      data, effect, point);
}

void WindowImpl::OnDragLeave(IDataObject* data) {
  return delegate_->GetContentView()->GetNative()->OnDragLeave(data);
}

int WindowImpl::OnDrop(IDataObject* data, int effect, const Point& point) {
  return delegate_->GetContentView()->GetNative()->OnDrop(data, effect, point);
}

void WindowImpl::TrackMouse(bool enable) {
  TRACKMOUSEEVENT event = {0};
  event.cbSize = sizeof(event);
  event.hwndTrack = hwnd();
  event.dwFlags = (enable ? 0 : TME_CANCEL) | TME_LEAVE;
  event.dwHoverTime = 0;
  TrackMouseEvent(&event);
}

bool WindowImpl::GetClientAreaInsets(Insets* insets) {
  // Returning false causes the default handling in OnNCCalcSize() to
  // be invoked.
  if (delegate_->HasFrame())
    return false;

  if (IsMaximized()) {
    // Windows automatically adds a standard width border to all sides when a
    // window is maximized.
    int border_thickness = GetFrameThickness(scale_factor());
    *insets = Insets(border_thickness, border_thickness,
                     border_thickness, border_thickness);
    return true;
  }

  *insets = Insets();
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Public Window API implementation.

void Window::PlatformInit(const Options& options) {
  window_ = new WindowImpl(options, this);

  YGConfigSetPointScaleFactor(yoga_config_,
                              GetScaleFactorForHWND(window_->hwnd()));
}

void Window::PlatformDestroy() {
  delete window_;
}

void Window::Close() {
  ::SendMessage(window_->hwnd(), WM_CLOSE, 0, 0);
}

void Window::SetHasShadow(bool has) {
  window_->SetHasShadow(has);
}

bool Window::HasShadow() const {
  return window_->has_shadow();
}

void Window::PlatformSetContentView(View* view) {
  if (content_view_)
    content_view_->GetNative()->BecomeContentView(nullptr);
  view->GetNative()->BecomeContentView(window_);
  view->SetPixelBounds(Rect(window_->GetContentPixelBounds().size()));
}

void Window::SetContentSize(const SizeF& size) {
  Size psize = ToRoundedSize(ScaleSize(size, window_->scale_factor()));
  if (HasFrame())
    psize = ContentToWindowSize(window_, !!menu_bar_, psize);
  ::SetWindowPos(window_->hwnd(), NULL, 0, 0, psize.width(), psize.height(),
                 SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
}

void Window::SetBounds(const RectF& bounds) {
  window_->SetPixelBounds(
      ToNearestRect(ScaleRect(bounds, window_->scale_factor())));
}

RectF Window::GetBounds() const {
  return ScaleRect(RectF(window_->GetPixelBounds()),
                         1.0f / window_->scale_factor());
}

void Window::SetSizeConstraints(const SizeF& min_size, const SizeF& max_size) {
  window_->set_min_size(ToRoundedSize(ScaleSize(min_size,
                                                window_->scale_factor())));
  window_->set_max_size(ToRoundedSize(ScaleSize(max_size,
                                                window_->scale_factor())));
}

std::tuple<SizeF, SizeF> Window::GetSizeConstraints() const {
  return std::make_tuple(ScaleSize(SizeF(window_->min_size()),
                                   1.f / window_->scale_factor()),
                         ScaleSize(SizeF(window_->max_size()),
                                   1.f / window_->scale_factor()));
}

void Window::SetContentSizeConstraints(const SizeF& min_size,
                                       const SizeF& max_size) {
  if (!HasFrame()) {
    SetSizeConstraints(min_size, max_size);
    return;
  }
  if (min_size.IsEmpty())
    window_->set_min_size(Size());
  else
    window_->set_min_size(ContentToWindowSize(
        window_,  !!menu_bar_,
        ToRoundedSize(ScaleSize(min_size, window_->scale_factor()))));
  if (max_size.IsEmpty())
    window_->set_max_size(Size());
  else
    window_->set_max_size(ContentToWindowSize(
        window_,  !!menu_bar_,
        ToRoundedSize(ScaleSize(max_size, window_->scale_factor()))));
}

std::tuple<SizeF, SizeF> Window::GetContentSizeConstraints() const {
  if (!HasFrame())
    return GetSizeConstraints();
  return std::make_tuple(
      window_->min_size().IsEmpty() ?
          SizeF() :
          WindowToContentSize(window_, !!menu_bar_,
                              ScaleSize(SizeF(window_->min_size()),
                                        1.f / window_->scale_factor())),
      window_->max_size().IsEmpty() ?
          SizeF() :
          WindowToContentSize(window_, !!menu_bar_,
                              ScaleSize(SizeF(window_->max_size()),
                                        1.f / window_->scale_factor())));
}

void Window::Activate() {
  HWND hwnd = window_->hwnd();
  ::ShowWindow(hwnd, SW_SHOW);
  ::SetForegroundWindow(hwnd);
}

void Window::Deactivate() {
  HWND hwnd = ::GetNextWindow(window_->hwnd(), GW_HWNDNEXT);
  if (hwnd)
    ::SetForegroundWindow(hwnd);
}

bool Window::IsActive() const {
  return ::GetActiveWindow() == window_->hwnd();
}

void Window::SetVisible(bool visible) {
  ::ShowWindow(window_->hwnd(), visible ? SW_SHOWNOACTIVATE : SW_HIDE);
}

bool Window::IsVisible() const {
  return !!::IsWindowVisible(window_->hwnd());
}

void Window::SetAlwaysOnTop(bool top) {
  ::SetWindowPos(window_->hwnd(), top ? HWND_TOPMOST : HWND_NOTOPMOST,
                 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

bool Window::IsAlwaysOnTop() const {
  return (::GetWindowLong(window_->hwnd(), GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;
}

void Window::SetFullscreen(bool fullscreen) {
  window_->SetFullscreen(fullscreen);
}

bool Window::IsFullscreen() const {
  return window_->IsFullscreen();
}

void Window::Maximize() {
  ::ShowWindow(window_->hwnd(), SW_MAXIMIZE);
}

void Window::Unmaximize() {
  ::ShowWindow(window_->hwnd(), SW_RESTORE);
}

bool Window::IsMaximized() const {
  return window_->IsMaximized();
}

void Window::Minimize() {
  ::ShowWindow(window_->hwnd(), SW_MINIMIZE);
}

void Window::Restore() {
  ::ShowWindow(window_->hwnd(), SW_RESTORE);
}

bool Window::IsMinimized() const {
  return !!::IsIconic(window_->hwnd());
}

void Window::SetResizable(bool yes) {
  window_->SetWindowStyle(WS_THICKFRAME, yes);
}

bool Window::IsResizable() const {
  return window_->HasWindowStyle(WS_THICKFRAME);
}

void Window::SetMaximizable(bool yes) {
  window_->SetWindowStyle(WS_MAXIMIZEBOX, yes);
}

bool Window::IsMaximizable() const {
  return window_->HasWindowStyle(WS_MAXIMIZEBOX);
}

void Window::SetMinimizable(bool minimizable) {
  window_->SetWindowStyle(WS_MINIMIZEBOX, minimizable);
}

bool Window::IsMinimizable() const {
  return window_->HasWindowStyle(WS_MINIMIZEBOX);
}

void Window::SetMovable(bool movable) {
  UINT flags = MF_BYCOMMAND | (movable ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
  ::EnableMenuItem(::GetSystemMenu(window_->hwnd(), FALSE), SC_MOVE, flags);
}

bool Window::IsMovable() const {
  MENUITEMINFO mii = {0};
  mii.cbSize = sizeof(MENUITEMINFO);
  HMENU menu = ::GetSystemMenu(window_->hwnd(), FALSE);
  ::GetMenuItemInfo(menu, SC_MOVE, FALSE, &mii);
  return (mii.fState & MFS_DISABLED) == 0;
}

void Window::SetTitle(const std::string& title) {
  ::SetWindowTextW(window_->hwnd(), base::UTF8ToWide(title).c_str());
}

std::string Window::GetTitle() const {
  return base::WideToUTF8(GetWindowString(window_->hwnd()));
}

void Window::SetBackgroundColor(Color color) {
  window_->SetBackgroundColor(color);
}

void Window::SetSkipTaskbar(bool skip) {
  Microsoft::WRL::ComPtr<ITaskbarList> taskbar;
  if (FAILED(::CoCreateInstance(CLSID_TaskbarList, nullptr,
                                CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&taskbar))) ||
      FAILED(taskbar->HrInit()))
    return;
  if (skip)
    taskbar->DeleteTab(window_->hwnd());
  else
    taskbar->AddTab(window_->hwnd());
}

void Window::PlatformSetIcon(Image* icon) {
  SizeF big_size(::GetSystemMetrics(SM_CXICON),
                 ::GetSystemMetrics(SM_CYICON));
  hicon_ = icon->GetHICON(big_size);
  LPARAM param = reinterpret_cast<LPARAM>(hicon_.get());
  ::SendMessage(window_->hwnd(), WM_SETICON, ICON_BIG, param);
  ::SendMessage(window_->hwnd(), WM_SETICON, ICON_SMALL, param);
}

void Window::PlatformSetMenuBar(MenuBar* menu_bar) {
  ::SetMenu(window_->hwnd(), menu_bar ? menu_bar->GetNative() : NULL);
}

void Window::PlatformAddChildWindow(Window* child) {
  ::SetParent(child->GetNative()->hwnd(), window_->hwnd());
}

void Window::PlatformRemoveChildWindow(Window* child) {
  ::SetParent(child->GetNative()->hwnd(), NULL);
}

}  // namespace nu
