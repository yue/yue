// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#include "nativeui/container.h"
#include "nativeui/menu_bar.h"
#include "third_party/yoga/yoga/Yoga.h"

namespace nu {

Window::Window(const Options& options)
    : has_frame_(options.frame),
      yoga_config_(YGConfigNew()) {
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

void Window::SetContentView(View* view) {
  if (!view) {
    LOG(ERROR) << "Content view can not be null";
    return;
  }
  PlatformSetContentView(view);
  content_view_ = view;
}

View* Window::GetContentView() const {
  return content_view_.get();
}

#if defined(OS_WIN) || defined(OS_LINUX)
void Window::SetMenu(MenuBar* menu_bar) {
  PlatformSetMenu(menu_bar);
  menu_bar_ = menu_bar;
}

MenuBar* Window::GetMenu() const {
  return menu_bar_.get();
}
#endif

}  // namespace nu
