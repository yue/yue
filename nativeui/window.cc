// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#include "nativeui/container.h"

namespace nu {

Window::Window(const Options& options) : content_view_(new Container) {
  PlatformInit(options);
  PlatformSetContentView(content_view_.get());
}

void Window::SetContentView(View* view) {
  PlatformSetContentView(view);
  content_view_ = view;
}

View* Window::GetContentView() const {
  return content_view_.get();
}

}  // namespace nu
