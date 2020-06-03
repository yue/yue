// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/focus_manager.h"

#include "nativeui/win/container_win.h"

namespace nu {

FocusManager::FocusManager() {
}

FocusManager::~FocusManager() {
}

void FocusManager::RemoveFocus(ViewImpl* view) {
  if (view == focused_view_)
    focused_view_ = nullptr;
}

void FocusManager::TakeFocus(ViewImpl* view) {
  // This happens when you switch to a child window.
  if (focused_view_ == view)
    return;

  if (focused_view_)
    focused_view_->SetFocus(false);
  focused_view_ = view;
}

void FocusManager::AdvanceFocus(ViewImpl* view, bool reverse) {
  // Just focus the view if it is not a container.
  if (view->type() < ControlType::Container) {
    view->SetFocus(true);
    return;
  }

  // Should we focus on next view.
  bool focus_on_next_view = focused_view_ == nullptr;

  ContainerImpl* container = static_cast<ContainerImpl*>(view);
  bool found = DoAdvanceFocus(container, reverse, &focus_on_next_view);

  // Search again from start.
  if (!found && focus_on_next_view && focused_view_) {
    focused_view_->SetFocus(false);
    focused_view_ = nullptr;
    DoAdvanceFocus(container, reverse, &focus_on_next_view);
  }
}

bool FocusManager::DoAdvanceFocus(ContainerImpl* container, bool reverse,
                                  bool* focus_on_next_view) {
  bool result = false;
  // Iterate views recusively.
  container->adapter()->ForEach([&](ViewImpl* child) {
    if (!child->is_visible())
      return true;  // continue

    if (child->type() >= ControlType::Container &&
        DoAdvanceFocus(static_cast<ContainerImpl*>(child), reverse,
                       focus_on_next_view)) {
      result = true;
      return false;  // break
    }

    if (child == focused_view_) {
      *focus_on_next_view = true;
    } else if (*focus_on_next_view && child->is_focusable()) {
      if (focused_view_)
        focused_view_->SetFocus(false);
      focused_view_ = child;
      child->SetFocus(true);
      result = true;
      return false;  // break
    }
    return true;  // continue
  }, reverse);
  return result;
}

}  // namespace nu
