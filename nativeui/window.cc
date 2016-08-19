// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#include "nativeui/container.h"

namespace nu {

Window::Window(const Options& options) {
  PlatformInit(options);
  SetContentView(new Container);
}

void Window::SetContentView(Container* container) {
  PlatformSetContentView(container);
  content_view_ = container;
  content_view_->Layout();
}

Container* Window::GetContentView() const {
  return content_view_.get();
}

}  // namespace nu
