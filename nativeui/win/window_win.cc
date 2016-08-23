// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#include "nativeui/win/base_view.h"
#include "nativeui/win/window_impl.h"

namespace nu {

namespace {

class TopLevelWindow : public WindowImpl {
 public:
  explicit TopLevelWindow(Window* delegate)
    : WindowImpl(), delegate_(delegate) {}

  void SetPixelBounds(const gfx::Rect& bounds) {
    SetWindowPos(hwnd(), NULL,
                 bounds.x(), bounds.y(), bounds.width(), bounds.height(),
                 SWP_NOACTIVATE | SWP_NOZORDER);
    RedrawWindow(hwnd(), NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
  }

  gfx::Rect GetPixelBounds() {
    RECT r;
    GetWindowRect(hwnd(), &r);
    return gfx::Rect(r);
  }

  gfx::Rect GetContentPixelBounds() {
    RECT r;
    GetClientRect(hwnd(), &r);
    POINT point = { r.left, r.top };
    ClientToScreen(hwnd(), &point);
    return gfx::Rect(point.x, point.y, r.right - r.left, r.bottom - r.top);
  }

 protected:
  CR_BEGIN_MSG_MAP_EX(TopLevelWindow, WindowImpl)
    CR_MSG_WM_SIZE(OnSize)
    CR_MSG_WM_PAINT(OnPaint)
    CR_MESSAGE_HANDLER_EX(WM_SETCURSOR, OnSetCursor);
  CR_END_MSG_MAP()

 private:
  void OnSize(UINT param, const gfx::Size& size);
  void OnPaint(HDC dc);
  LRESULT OnSetCursor(UINT message, WPARAM w_param, LPARAM l_param);

  Window* delegate_;
};

void TopLevelWindow::OnSize(UINT param, const gfx::Size& size) {
  if (delegate_->GetContentView())
    delegate_->GetContentView()->view()->SetPixelBounds(
        gfx::Rect(gfx::Point(), size));
}

void TopLevelWindow::OnPaint(HDC dc) {
  PAINTSTRUCT ps;
  BeginPaint(hwnd(), &ps);
  EndPaint(hwnd(), &ps);
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
gfx::Rect ContentToWindowBounds(WindowImpl* window, const gfx::Rect& bounds) {
  RECT rect = bounds.ToRECT();
  AdjustWindowRectEx(&rect, window->window_style(),
                     FALSE, window->window_ex_style());
  return gfx::Rect(rect);
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

void Window::PlatformSetContentView(Container* container) {
  container->view()->BecomeContentView(window_);
}

void Window::SetContentBounds(const gfx::Rect& bounds) {
  TopLevelWindow* win = static_cast<TopLevelWindow*>(window_);
  gfx::Rect pixel_bounds(ScaleToEnclosingRect(bounds, win->scale_factor()));
  win->SetPixelBounds(ContentToWindowBounds(win, pixel_bounds));
}

gfx::Rect Window::GetContentBounds() const {
  TopLevelWindow* win = static_cast<TopLevelWindow*>(window_);
  return ScaleToEnclosingRect(win->GetContentPixelBounds(),
                              1.0f / win->scale_factor());
}

void Window::SetBounds(const gfx::Rect& bounds) {
  TopLevelWindow* win = static_cast<TopLevelWindow*>(window_);
  win->SetPixelBounds(ScaleToEnclosingRect(bounds, win->scale_factor()));
}

gfx::Rect Window::GetBounds() const {
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
