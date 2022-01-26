// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#include <utility>

#include "base/logging.h"
#include "nativeui/container.h"
#include "nativeui/menu_bar.h"
#include "nativeui/screen.h"
#include "third_party/yoga/Yoga.h"

#if defined(OS_MAC)
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
  YGConfigFree(yoga_config_);
  content_view_->BecomeContentView();
}

void Window::SetContentView(scoped_refptr<View> view) {
  CHECK(view) << "Content view can not be null";
  if (content_view_)
    content_view_->BecomeContentView();
  PlatformSetContentView(view.get());
  content_view_ = std::move(view);
  content_view_->BecomeContentView(this);
}

View* Window::GetContentView() const {
  return content_view_.get();
}

#if defined(OS_WIN) || defined(OS_LINUX)
void Window::Center() {
  Display display = Screen::GetCurrent()->GetDisplayNearestWindow(this);
  if (display.work_area.IsEmpty())
    return;
  RectF bounds = GetBounds();
  if (bounds.IsEmpty())
    return;
  bounds.set_x((display.work_area.width() - bounds.width()) / 2);
  bounds.set_y((display.work_area.height() - bounds.height()) / 2);
  SetBounds(bounds);
}
#endif

#if defined(OS_WIN) || defined(OS_MAC)
SizeF Window::GetContentSize() const {
  return content_view_->GetBounds().size();
}
#endif

#if defined(OS_WIN) || defined(OS_LINUX)
void Window::SetIcon(scoped_refptr<Image> icon) {
  PlatformSetIcon(icon.get());
  icon_ = std::move(icon);
}

void Window::SetMenuBar(scoped_refptr<MenuBar> menu_bar) {
  if (menu_bar_)
    menu_bar_->SetWindow(nullptr);
  PlatformSetMenuBar(menu_bar.get());
  menu_bar_ = std::move(menu_bar);
  menu_bar_->SetWindow(this);
}
#endif

void Window::AddChildWindow(scoped_refptr<Window> child) {
  if (child->GetParentWindow())
    return;
  auto it = std::find(child_windows_.begin(), child_windows_.end(),
                      child.get());
  if (it == child_windows_.end()) {
    child->parent_ = this;
    PlatformAddChildWindow(child.get());
    child_windows_.emplace_back(std::move(child));
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

void Window::NotifyWindowClosed() {
  DCHECK(!is_closed_);
  is_closed_ = true;
  for (const auto& i : child_windows_) {
    i->should_close = nullptr;  // don't give user a chance to cancel.
    i->Close();
  }
  on_close.Emit(this);
}

}  // namespace nu
