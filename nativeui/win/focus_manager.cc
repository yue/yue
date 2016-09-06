// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/focus_manager.h"

#include "nativeui/group.h"
#include "nativeui/win/base_view.h"

namespace nu {

FocusManager::FocusManager() {
}

FocusManager::~FocusManager() {
}

void FocusManager::TakeFocus(View* view) {
  if (focused_view_)
    focused_view_->view()->SetFocus(false);
  focused_view_ = view;
}

void FocusManager::AdvanceFocus(Container* container, bool reverse) {
  // Should we focus on next view.
  bool focus_on_next_view = focused_view_ == nullptr;

  DoAdvanceFocus(container, reverse, &focus_on_next_view);
}

bool FocusManager::DoAdvanceFocus(Container* container, bool reverse,
                                  bool* focus_on_next_view) {
  // Iterate views recusively.
  for (int i = reverse ? container->child_count() - 1 : 0;
       reverse ? (i >= 0) : (i < container->child_count());
       reverse ? --i : ++i) {
    View* child = container->child_at(i);
    if (child->view()->type() == ControlType::Container &&
        DoAdvanceFocus(static_cast<Container*>(child), reverse,
                       focus_on_next_view)) {
      return true;
    } else if (child->view()->type() == ControlType::Group &&
               DoAdvanceFocus(static_cast<Group*>(child)->GetContentView(),
                              reverse, focus_on_next_view)) {
      return true;
    }

    if (child == focused_view_) {
      *focus_on_next_view = true;
    } else if (*focus_on_next_view && child->view()->CanHaveFocus()) {
      if (focused_view_)
        focused_view_->view()->SetFocus(false);
      focused_view_ = child;
      child->view()->SetFocus(true);
      return true;
    }
  }
  return false;
}

}  // namespace nu
