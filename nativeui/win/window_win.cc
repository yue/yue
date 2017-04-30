// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/window_win.h"

#include <memory>

#include "nativeui/accelerator.h"
#include "nativeui/accelerator_manager.h"
#include "nativeui/events/event.h"
#include "nativeui/events/win/event_win.h"
#include "nativeui/gfx/geometry/rect_conversions.h"
#include "nativeui/gfx/win/double_buffer.h"
#include "nativeui/gfx/win/painter_win.h"
#include "nativeui/gfx/win/screen_win.h"
#include "nativeui/menu_bar.h"
#include "nativeui/win/menu_base_win.h"
#include "nativeui/win/subwin_view.h"
#include "nativeui/win/util/hwnd_util.h"
#include "third_party/yoga/yoga/Yoga.h"

namespace nu {

namespace {

// Convert between window and client areas.
Rect ContentToWindowBounds(Win32Window* window, bool has_menu_bar,
                           const Rect& bounds) {
  RECT rect = bounds.ToRECT();
  AdjustWindowRectEx(&rect, window->window_style(), has_menu_bar,
                     window->window_ex_style());
  return Rect(rect);
}

bool IsShiftPressed() {
  return (::GetKeyState(VK_SHIFT) & 0x8000) == 0x8000;
}

}  // namespace

WindowImpl::WindowImpl(Window* delegate)
    : delegate_(delegate),
      scale_factor_(GetScaleFactorForHWND(hwnd())) {
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
  if (delegate_->GetContentView()->GetClassName() == Container::kClassName)
    focus_manager_.AdvanceFocus(
        static_cast<Container*>(delegate_->GetContentView()), IsShiftPressed());
}

void WindowImpl::SetCapture(ViewImpl* view) {
  captured_view_ = view;
  ::SetCapture(hwnd());
}

void WindowImpl::ReleaseCapture() {
  if (::GetCapture() == hwnd())
    ::ReleaseCapture();
}

void WindowImpl::SetBackgroundColor(nu::Color color) {
  background_color_ = color;
  InvalidateRect(hwnd(), NULL, TRUE);
}

void WindowImpl::OnCaptureChanged(HWND window) {
  if (captured_view_) {
    captured_view_->OnCaptureLost();
    captured_view_ = nullptr;
  }
}

void WindowImpl::OnClose() {
  if (delegate_->should_close.is_null() || delegate_->should_close.Run()) {
    delegate_->on_close.Emit();
    SetMsgHandled(false);
  }
}

void WindowImpl::OnCommand(UINT code, int command, HWND window) {
  if (!code && !window && delegate_->GetMenu()) {
    DispatchCommandToItem(delegate_->GetMenu(), command);
    return;
  } else if (::GetParent(window) != hwnd()) {
    LOG(ERROR) << "Received notification " << code << " " << command
               << " from a non-child window";
    return;
  }

  auto* control = reinterpret_cast<SubwinView*>(GetWindowUserData(window));
  control->OnCommand(code, command);
}

HBRUSH WindowImpl::OnCtlColorStatic(HDC dc, HWND window) {
  auto* control = reinterpret_cast<SubwinView*>(GetWindowUserData(window));
  HBRUSH brush = NULL;
  SetMsgHandled(control->OnCtlColor(dc, &brush));
  return brush;
}

void WindowImpl::OnSize(UINT param, const Size& size) {
  if (!delegate_->GetContentView())
    return;
  delegate_->GetContentView()->GetNative()->SizeAllocate(Rect(size));
  RedrawWindow(hwnd(), NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
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

BOOL WindowImpl::OnMouseWheel(bool vertical, UINT flags, int delta,
                                  const Point& point) {
  POINT p = point.ToPOINT();
  ScreenToClient(hwnd(), &p);
  return delegate_->GetContentView()->GetNative()->OnMouseWheel(
      vertical, flags, delta, Point(p));
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
      focus_manager()->focused_view()->GetNative()->OnKeyEvent(&msg))
    return 0;

  // If no one handles it then pass the event to menu.
  KeyEvent event(&msg, delegate_->GetContentView()->GetNative());
  if (event.type == EventType::KeyDown && delegate_->GetMenu()) {
    Accelerator accelerator(event);
    int command = delegate_->GetMenu()->accel_manager()->Process(accelerator);
    if (command != -1) {
      DispatchCommandToItem(delegate_->GetMenu(), command);
      return 0;
    }
  }
  SetMsgHandled(false);
  return 0;
}

void WindowImpl::OnChar(UINT ch, UINT repeat, UINT flags) {
  if (ch == VK_TAB)
    AdvanceFocus();
}

void WindowImpl::OnPaint(HDC) {
  PAINTSTRUCT ps;
  BeginPaint(hwnd(), &ps);

  Rect bounds(GetContentPixelBounds());
  Rect dirty(ps.rcPaint);
  base::win::ScopedGetDC dc(hwnd());
  {
    // Double buffering the drawing.
    DoubleBuffer buffer(dc, bounds.size(), dirty, dirty.origin());

    // Background.
    PainterWin painter(buffer.dc(), scale_factor_);
    painter.SetColor(background_color_);
    painter.FillRectPixel(dirty);

    // Draw.
    delegate_->GetContentView()->GetNative()->Draw(&painter, dirty);
  }

  EndPaint(hwnd(), &ps);
}

LRESULT WindowImpl::OnEraseBkgnd(HDC dc) {
  // Needed to prevent resize flicker.
  return 1;
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

LRESULT WindowImpl::OnNCHitTest(UINT msg, WPARAM w_param, LPARAM l_param) {
  // Only override this for frameless window.
  if (delegate_->HasFrame()) {
    SetMsgHandled(false);
    return 0;
  }

  // Do nothing for non-client area for now.
  LRESULT hit = DefWindowProc(hwnd(), msg, w_param, l_param);
  if (hit != HTCLIENT)
    return hit;

  // Convert point to client area.
  POINT temp = { CR_GET_X_LPARAM(l_param), CR_GET_Y_LPARAM(l_param) };
  ::MapWindowPoints(HWND_DESKTOP, hwnd(), &temp, 1);
  Point point(temp);

  return delegate_->GetContentView()->GetNative()->HitTest(point);
}

void WindowImpl::TrackMouse(bool enable) {
  TRACKMOUSEEVENT event = {0};
  event.cbSize = sizeof(event);
  event.hwndTrack = hwnd();
  event.dwFlags = (enable ? 0 : TME_CANCEL) | TME_LEAVE;
  event.dwHoverTime = 0;
  TrackMouseEvent(&event);
}

///////////////////////////////////////////////////////////////////////////////
// Public Window API implementation.

void Window::PlatformInit(const Options& options) {
  window_ = new WindowImpl(this);
  if (!options.bounds.IsEmpty())
    SetBounds(options.bounds);

  YGConfigSetPointScaleFactor(yoga_config_,
                              GetScaleFactorForHWND(window_->hwnd()));
}

void Window::PlatformDestroy() {
  delete window_;
}

void Window::Close() {
  ::SendMessage(window_->hwnd(), WM_CLOSE, 0, 0);
}

void Window::PlatformSetContentView(View* view) {
  view->GetNative()->BecomeContentView(window_);
  view->SetPixelBounds(Rect(window_->GetContentPixelBounds().size()));
}

void Window::PlatformSetMenu(MenuBar* menu_bar) {
  ::SetMenu(window_->hwnd(), menu_bar ? menu_bar->GetNative() : NULL);
}

void Window::SetContentBounds(const RectF& bounds) {
  Rect pixels = ToNearestRect(ScaleRect(bounds, window_->scale_factor()));
  window_->SetPixelBounds(ContentToWindowBounds(window_, !!menu_bar_, pixels));
}

RectF Window::GetContentBounds() const {
  return ScaleRect(RectF(window_->GetContentPixelBounds()),
                   1.0f / window_->scale_factor());
}

void Window::SetBounds(const RectF& bounds) {
  window_->SetPixelBounds(
      ToNearestRect(ScaleRect(bounds, window_->scale_factor())));
}

RectF Window::GetBounds() const {
  return ScaleRect(RectF(window_->GetPixelBounds()),
                         1.0f / window_->scale_factor());
}

void Window::SetVisible(bool visible) {
  ShowWindow(window_->hwnd(), visible ? SW_SHOWNOACTIVATE : SW_HIDE);
}

bool Window::IsVisible() const {
  return !!::IsWindowVisible(window_->hwnd());
}

void Window::SetBackgroundColor(Color color) {
  window_->SetBackgroundColor(color);
}

}  // namespace nu
