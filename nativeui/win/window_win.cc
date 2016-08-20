// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#include "nativeui/win/window_impl.h"

namespace nu {

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
  window_ = new WindowImpl;
  window_->SetBounds(options.content_bounds);
}

void Window::PlatformSetContentView(Container* container) {
}

}  // namespace nu
