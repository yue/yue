// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#include "nativeui/gfx/win/double_buffer.h"
#include "nativeui/win/subwin_view.h"
#include "nativeui/win/util/hwnd_util.h"

namespace nu {

namespace {

class TopLevelWindow : public WindowImpl {
 public:
  explicit TopLevelWindow(Window* delegate) : delegate_(delegate) {}

  void SetPixelBounds(const Rect& bounds);
  Rect GetPixelBounds();
  Rect GetContentPixelBounds();

 protected:
  CR_BEGIN_MSG_MAP_EX(TopLevelWindow, WindowImpl)
    CR_MSG_WM_CLOSE(OnClose)
    CR_MSG_WM_COMMAND(OnCommand)
    CR_MSG_WM_SIZE(OnSize)
    CR_MSG_WM_MOUSEMOVE(OnMouseMove)
    CR_MSG_WM_MOUSELEAVE(OnMouseLeave)
    CR_MSG_WM_PAINT(OnPaint)
    CR_MSG_WM_ERASEBKGND(OnEraseBkgnd)
  CR_END_MSG_MAP()

 private:
  void OnClose();
  void OnCommand(UINT code, int command, HWND window);
  void OnSize(UINT param, const Size& size);
  void OnMouseMove(UINT param, const Point& point);
  void OnMouseLeave();
  void OnPaint(HDC dc);
  LRESULT OnEraseBkgnd(HDC dc);

  void TrackMouse(bool enable);

  bool mouse_in_window_ = false;
  Window* delegate_;
};

void TopLevelWindow::SetPixelBounds(const Rect& bounds) {
  SetWindowPos(hwnd(), NULL,
               bounds.x(), bounds.y(), bounds.width(), bounds.height(),
               SWP_NOACTIVATE | SWP_NOZORDER);
}

Rect TopLevelWindow::GetPixelBounds() {
  RECT r;
  GetWindowRect(hwnd(), &r);
  return Rect(r);
}

Rect TopLevelWindow::GetContentPixelBounds() {
  RECT r;
  GetClientRect(hwnd(), &r);
  POINT point = { r.left, r.top };
  ClientToScreen(hwnd(), &point);
  return Rect(point.x, point.y, r.right - r.left, r.bottom - r.top);
}

void TopLevelWindow::OnClose() {
  if (delegate_->should_close.is_null() || delegate_->should_close.Run()) {
    delegate_->on_close.Emit();
    SetMsgHandled(FALSE);
  }
}

void TopLevelWindow::OnCommand(UINT code, int command, HWND window) {
  if (::GetParent(window) != hwnd()) {
    LOG(ERROR) << "Received notification " << code << " " << command
               << "from a non-child window";
    return;
  }

  auto* control = reinterpret_cast<SubwinView*>(GetWindowUserData(window));
  control->OnCommand(code, command);
}

void TopLevelWindow::OnSize(UINT param, const Size& size) {
  if (!delegate_->GetContentView())
    return;
  delegate_->GetContentView()->view()->SetPixelBounds(Rect(size));
  RedrawWindow(hwnd(), NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void TopLevelWindow::OnMouseMove(UINT param, const Point& point) {
  if (!mouse_in_window_) {
    mouse_in_window_ = true;
    delegate_->GetContentView()->view()->OnMouseEnter();
    TrackMouse(true);
  }
  delegate_->GetContentView()->view()->OnMouseMove(point);
}

void TopLevelWindow::OnMouseLeave() {
  TrackMouse(false);
  mouse_in_window_ = false;
  delegate_->GetContentView()->view()->OnMouseLeave();
}

void TopLevelWindow::OnPaint(HDC) {
  PAINTSTRUCT ps;
  BeginPaint(hwnd(), &ps);

  Rect bounds(GetContentPixelBounds());
  Rect dirty(ps.rcPaint);
  base::win::ScopedGetDC dc(hwnd());
  {
    // Double buffering the drawing.
    DoubleBuffer buffer(dc, bounds.size(), dirty, dirty.origin());

    // Background.
    Gdiplus::Graphics graphics(buffer.dc());
    Gdiplus::SolidBrush solid_brush(Gdiplus::Color(255, 255, 255, 255));
    graphics.FillRectangle(&solid_brush,
                           dirty.x(), dirty.y(),
                           dirty.width(), dirty.height());

    // Draw.
    delegate_->GetContentView()->view()->Draw(&graphics, dirty);
  }

  EndPaint(hwnd(), &ps);
}

LRESULT TopLevelWindow::OnEraseBkgnd(HDC dc) {
  // Needed to prevent resize flicker.
  return 1;
}

void TopLevelWindow::TrackMouse(bool enable) {
  TRACKMOUSEEVENT event = {0};
  event.cbSize = sizeof(event);
  event.hwndTrack = hwnd();
  event.dwFlags = (enable ? 0 : TME_CANCEL) | TME_LEAVE;
  event.dwHoverTime = 0;
  TrackMouseEvent(&event);
}

// Convert between window and client areas.
Rect ContentToWindowBounds(WindowImpl* window, const Rect& bounds) {
  RECT rect = bounds.ToRECT();
  AdjustWindowRectEx(&rect, window->window_style(),
                     FALSE, window->window_ex_style());
  return Rect(rect);
}

}  // namespace

Window::~Window() {
  delete window_;
}

void Window::PlatformInit(const Options& options) {
  TopLevelWindow* win = new TopLevelWindow(this);
  window_ = win;

  if (!options.bounds.IsEmpty())
    SetBounds(options.bounds);
}

void Window::Close() {
  ::SendMessage(window_->hwnd(), WM_CLOSE, 0, 0);
}

void Window::PlatformSetContentView(Container* container) {
  container->view()->BecomeContentView(window_);
  container->Layout();
}

void Window::SetContentBounds(const Rect& bounds) {
  TopLevelWindow* win = static_cast<TopLevelWindow*>(window_);
  Rect pixel_bounds(ScaleToEnclosingRect(bounds, win->scale_factor()));
  win->SetPixelBounds(ContentToWindowBounds(win, pixel_bounds));
}

Rect Window::GetContentBounds() const {
  TopLevelWindow* win = static_cast<TopLevelWindow*>(window_);
  return ScaleToEnclosingRect(win->GetContentPixelBounds(),
                              1.0f / win->scale_factor());
}

void Window::SetBounds(const Rect& bounds) {
  TopLevelWindow* win = static_cast<TopLevelWindow*>(window_);
  win->SetPixelBounds(ScaleToEnclosingRect(bounds, win->scale_factor()));
}

Rect Window::GetBounds() const {
  TopLevelWindow* win = static_cast<TopLevelWindow*>(window_);
  return ScaleToEnclosingRect(win->GetPixelBounds(),
                              1.0f / win->scale_factor());
}

void Window::SetVisible(bool visible) {
  ShowWindow(window_->hwnd(), visible ? SW_SHOWNOACTIVATE : SW_HIDE);
}

bool Window::IsVisible() const {
  return !!::IsWindowVisible(window_->hwnd());
}

}  // namespace nu
