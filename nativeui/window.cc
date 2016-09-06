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
  if (!container) {
    LOG(ERROR) << "Content view can not be null";
    return;
  }
  content_view_ = container;
  PlatformSetContentView(container);
}

Container* Window::GetContentView() const {
  return content_view_.get();
}

}  // namespace nu
