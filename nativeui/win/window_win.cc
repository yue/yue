// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#include "nativeui/win/window_impl.h"

namespace nu {

namespace {

class TopLevelWindow : public WindowImpl {
 public:
  explicit TopLevelWindow(Window* delegate) : delegate_(delegate) {}

 protected:
  CR_BEGIN_MSG_MAP_EX(TopLevelWindow, WindowImpl)
    CR_MSG_WM_SIZE(OnSize)
  CR_END_MSG_MAP()

 private:
  void OnSize(UINT param, const gfx::Size& size);

  Window* delegate_;
};

void TopLevelWindow::OnSize(UINT param, const gfx::Size& size) {
  if (delegate_->GetContentView())
    delegate_->GetContentView()->view()->SetPixelBounds(
        gfx::Rect(gfx::Point(), size));
}

}  // namespace

Window::~Window() {
  delete window_;
}

void Window::SetVisible(bool visible) {
  ShowWindow(window_->hwnd(), visible ? SW_SHOWNOACTIVATE : SW_HIDE);
}

bool Window::IsVisible() const {
  return !!::IsWindowVisible(window_->hwnd());
}

void Window::PlatformInit(const Options& options) {
  window_ = new TopLevelWindow(this);
  window_->SetBounds(options.content_bounds);
}

void Window::PlatformSetContentView(Container* container) {
  SetParent(container->view()->hwnd(), window_->hwnd());
}

}  // namespace nu
