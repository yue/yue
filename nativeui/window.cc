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
}

Container* Window::GetContentView() const {
  return content_view_.get();
}

void Window::SetContentBounds(const gfx::Rect& bounds) {
  SetBounds(ContentBoundsToWindowBounds(bounds));
}

gfx::Rect Window::GetContentBounds() const {
  return WindowBoundsToContentBounds(GetBounds());
}

}  // namespace nu
