// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#include "nativeui/container.h"
#include "nativeui/menu_bar.h"
#include "third_party/yoga/yoga/Yoga.h"

#if defined(OS_MACOSX)
#include "nativeui/toolbar.h"
#endif

namespace nu {

Window::Window(const Options& options)
    : has_frame_(options.frame),
      transparent_(options.transparent),
      yoga_config_(YGConfigNew()) {
  // Initialize.
  PlatformInit(options);
  SetContentView(new Container);

  // Default window abilities.
  if (!options.frame && options.transparent) {
    SetResizable(false);
    SetMaximizable(false);
  }
}

Window::~Window() {
  PlatformDestroy();
}

void Window::SetContentView(View* view) {
  if (!view) {
    LOG(ERROR) << "Content view can not be null";
    return;
  }
  if (content_view_)
    content_view_->BecomeContentView(nullptr);
  PlatformSetContentView(view);
  content_view_ = view;
  content_view_->BecomeContentView(this);
}

View* Window::GetContentView() const {
  return content_view_.get();
}

SizeF Window::GetContentSize() const {
  return content_view_->GetBounds().size();
}

#if defined(OS_WIN) || defined(OS_LINUX)
void Window::SetMenuBar(MenuBar* menu_bar) {
  if (menu_bar_)
    menu_bar_->SetWindow(nullptr);
  PlatformSetMenuBar(menu_bar);
  menu_bar_ = menu_bar;
  menu_bar_->SetWindow(this);
}
#endif

void Window::AddChildWindow(Window* child) {
  if (child->GetParentWindow())
    return;
  auto it = std::find(child_windows_.begin(), child_windows_.end(), child);
  if (it == child_windows_.end()) {
    child->parent_ = this;
    child_windows_.emplace_back(child);
    PlatformAddChildWindow(child);
  }
}

void Window::RemoveChildWindow(Window* child) {
  if (child->GetParentWindow() != this)
    return;
  auto it = std::find(child_windows_.begin(), child_windows_.end(), child);
  if (it != child_windows_.end()) {
    PlatformRemoveChildWindow(child);
    child->parent_ = nullptr;
    child_windows_.erase(it);
  }
}

std::vector<Window*> Window::GetChildWindows() const {
  std::vector<Window*> result;
  result.reserve(child_windows_.size());
  for (const auto& i : child_windows_)
    result.push_back(i.get());
  return result;
}

void Window::CloseAllChildWindows() {
  for (const auto& i : child_windows_) {
    i->should_close = nullptr;  // don't give user a chance to cancel.
    i->Close();
  }
}

}  // namespace nu
