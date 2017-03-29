// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#include "nativeui/container.h"
#include "nativeui/menu_bar.h"
#include "nativeui/util/yoga.h"

namespace nu {

Window::Window(const Options& options) : yoga_config_(YGConfigNew()) {
  // Default yoga config.
  YGConfigSetExperimentalFeatureEnabled(yoga_config_,
                                        YGExperimentalFeatureRounding, true);

  // Initialize.
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

#if defined(OS_WIN) || defined(OS_LINUX)
void Window::SetMenuBar(MenuBar* menu_bar) {
  PlatformSetMenuBar(menu_bar);
  menu_bar_ = menu_bar;
}

MenuBar* Window::GetMenuBar() const {
  return menu_bar_.get();
}
#endif

}  // namespace nu
