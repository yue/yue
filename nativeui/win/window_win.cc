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
    return gfx::Rect(r);;
  }

  void SetBounds(const gfx::Rect& bounds) {
    SetPixelBounds(ScaleToEnclosingRect(bounds, scale_factor()));
  }

  gfx::Rect GetBounds() {
    return ScaleToEnclosingRect(GetPixelBounds(), 1.0f / scale_factor());
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

}  // namespace

Window::~Window() {
  delete window_;
}

void Window::PlatformInit(const Options& options) {
  TopLevelWindow* win = new TopLevelWindow(this);
  window_ = win;

  SetBounds(options.bounds);
}

void Window::PlatformSetContentView(Container* container) {
  container->view()->BecomeContentView(window_);
  container->Layout();
}

gfx::Rect Window::ContentBoundsToWindowBounds(const gfx::Rect& bounds) const {
  RECT rect = bounds.ToRECT();
  AdjustWindowRectEx(&rect, window_->window_style(),
                     FALSE, window_->window_ex_style());
  return gfx::Rect(rect);
}

gfx::Rect Window::WindowBoundsToContentBounds(const gfx::Rect& bounds) const {
  RECT rect;
  SetRectEmpty(&rect);
  AdjustWindowRectEx(&rect, window_->window_style(),
                     FALSE, window_->window_ex_style());
  gfx::Rect content_bounds(bounds);
  content_bounds.Subtract(gfx::Rect(rect));
  return content_bounds;
}

void Window::SetBounds(const gfx::Rect& bounds) {
  static_cast<TopLevelWindow*>(window_)->SetBounds(bounds);
}

gfx::Rect Window::GetBounds() const {
  return static_cast<TopLevelWindow*>(window_)->GetBounds();
}

void Window::SetVisible(bool visible) {
  ShowWindow(window_->hwnd(), visible ? SW_SHOWNOACTIVATE : SW_HIDE);
}

bool Window::IsVisible() const {
  return !!::IsWindowVisible(window_->hwnd());
}

}  // namespace nu
