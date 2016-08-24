// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#include "base/win/scoped_gdi_object.h"
#include "base/win/scoped_hdc.h"
#include "base/win/scoped_select_object.h"
#include "nativeui/win/base_view.h"
#include "nativeui/win/gdiplus.h"
#include "nativeui/win/window_impl.h"

namespace nu {

namespace {

class TopLevelWindow : public WindowImpl {
 public:
  explicit TopLevelWindow(Window* delegate)
    : WindowImpl(), delegate_(delegate) {}

  void SetPixelBounds(const gfx::Rect& bounds);
  gfx::Rect GetPixelBounds();
  gfx::Rect GetContentPixelBounds();

 protected:
  CR_BEGIN_MSG_MAP_EX(TopLevelWindow, WindowImpl)
    CR_MSG_WM_SIZE(OnSize)
    CR_MSG_WM_PAINT(OnPaint)
    CR_MSG_WM_ERASEBKGND(OnEraseBkgnd)
    CR_MESSAGE_HANDLER_EX(WM_SETCURSOR, OnSetCursor);
  CR_END_MSG_MAP()

 private:
  void OnSize(UINT param, const gfx::Size& size);
  void OnPaint(HDC dc);
  LRESULT OnEraseBkgnd(HDC dc);
  LRESULT OnSetCursor(UINT message, WPARAM w_param, LPARAM l_param);

  Window* delegate_;
};

void TopLevelWindow::SetPixelBounds(const gfx::Rect& bounds) {
  SetWindowPos(hwnd(), NULL,
               bounds.x(), bounds.y(), bounds.width(), bounds.height(),
               SWP_NOACTIVATE | SWP_NOZORDER);
}

gfx::Rect TopLevelWindow::GetPixelBounds() {
  RECT r;
  GetWindowRect(hwnd(), &r);
  return gfx::Rect(r);
}

gfx::Rect TopLevelWindow::GetContentPixelBounds() {
  RECT r;
  GetClientRect(hwnd(), &r);
  POINT point = { r.left, r.top };
  ClientToScreen(hwnd(), &point);
  return gfx::Rect(point.x, point.y, r.right - r.left, r.bottom - r.top);
}

void TopLevelWindow::OnSize(UINT param, const gfx::Size& size) {
  if (delegate_->GetContentView())
    delegate_->GetContentView()->view()->SetPixelBounds(
        gfx::Rect(gfx::Point(), size));
  RedrawWindow(hwnd(), NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void TopLevelWindow::OnPaint(HDC) {
  PAINTSTRUCT ps;
  BeginPaint(hwnd(), &ps);

  // Prepare double buffering.
  gfx::Rect bounds(GetContentPixelBounds());
  base::win::ScopedGetDC dc(hwnd());
  base::win::ScopedCreateDC mem_dc(CreateCompatibleDC(dc));
  base::win::ScopedBitmap mem_bitmap(
      CreateCompatibleBitmap(dc, bounds.width(), bounds.height()));
  base::win::ScopedSelectObject select_bitmap(mem_dc.Get(), mem_bitmap.get());

  // Background.
  Gdiplus::Graphics graphics(mem_dc.Get());
  Gdiplus::SolidBrush solid_brush(Gdiplus::Color(255, 255, 255, 255));
  graphics.FillRectangle(&solid_brush, 0, 0, bounds.width(), bounds.height());

  // Draw.
  gfx::Rect dirty(ps.rcPaint);
  delegate_->GetContentView()->view()->Draw(mem_dc.Get(), dirty);

  // Transfer the off-screen DC to the screen.
  BitBlt(dc, 0, 0, bounds.width(), bounds.height(),
         mem_dc.Get(), 0, 0, SRCCOPY);

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
  container->Layout();
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
