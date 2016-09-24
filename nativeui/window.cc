// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#include "nativeui/container.h"
#include "nativeui/menu_bar.h"

namespace nu {

Window::Window(const Options& options) {
  PlatformInit(options);
  SetContentView(new Container);
}

Window::~Window() {
  PlatformDestroy();
}

void Window::SetContentView(Container* container) {
  if (!container) {
    LOG(ERROR) << "Content view can not be null";
    return;
  }
  PlatformSetContentView(container);
  content_view_ = container;
}

Container* Window::GetContentView() const {
  return content_view_.get();
}

void Window::SetMenuBar(MenuBar* menu_bar) {
  PlatformSetMenuBar(menu_bar);
  menu_bar_ = menu_bar;
}

MenuBar* Window::GetMenuBar() const {
  return menu_bar_.get();
}

}  // namespace nu
