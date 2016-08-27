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
  explicit TopLevelWindow(Window* delegate)
    : WindowImpl(), delegate_(delegate) {}

  void SetPixelBounds(const Rect& bounds);
  Rect GetPixelBounds();
  Rect GetContentPixelBounds();

 protected:
  CR_BEGIN_MSG_MAP_EX(TopLevelWindow, WindowImpl)
    CR_MSG_WM_CLOSE(OnClose)
    CR_MSG_WM_COMMAND(OnCommand)
    CR_MSG_WM_SIZE(OnSize)
    CR_MSG_WM_PAINT(OnPaint)
    CR_MSG_WM_ERASEBKGND(OnEraseBkgnd)
    CR_MESSAGE_HANDLER_EX(WM_SETCURSOR, OnSetCursor);
  CR_END_MSG_MAP()

 private:
  void OnClose();
  void OnCommand(UINT code, int command, HWND window);
  void OnSize(UINT param, const Size& size);
  void OnPaint(HDC dc);
  LRESULT OnEraseBkgnd(HDC dc);
  LRESULT OnSetCursor(UINT message, WPARAM w_param, LPARAM l_param);

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
  delegate_->on_close.Notify();
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
  if (delegate_->GetContentView())
    delegate_->GetContentView()->view()->SetPixelBounds(
        Rect(Point(), size));
  RedrawWindow(hwnd(), NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
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
    graphics.FillRectangle(&solid_brush, 0, 0, bounds.width(), bounds.height());

    // Draw.
    delegate_->GetContentView()->view()->Draw(&graphics, dirty);
  }

  EndPaint(hwnd(), &ps);
}

LRESULT TopLevelWindow::OnEraseBkgnd(HDC dc) {
  // Needed to prevent resize flicker.
  return 1;
}

LRESULT TopLevelWindow::OnSetCursor(UINT message,
                                    WPARAM w_param, LPARAM l_param) {
  // Reimplement the necessary default behavior here. Calling DefWindowProc can
  // trigger weird non-client painting for non-glass windows with custom frames.
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
      SetMsgHandled(FALSE);
      break;
    default:
      // Use the default value, IDC_ARROW.
      break;
  }
  ::SetCursor(LoadCursor(NULL, cursor));
  return 1;
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
