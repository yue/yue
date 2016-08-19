// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#include "base/logging.h"

namespace nu {

void View::AddChildView(View* view) {
  if (view->parent_ == this)
    return;
  AddChildViewAt(view, child_count());
}

void View::AddChildViewAt(View* view, int index) {
  if (view == this || index < 0 || index > child_count())
    return;

  // TODO(zcbenz): support moving views.
  if (view->parent_) {
    LOG(ERROR) << "The view already has a parent.";
    return;
  }

  view->parent_ = this;
  children_.insert(children_.begin() + index, view);
  PlatformAddChildView(view);

  Layout();
}

void View::RemoveChildView(View* view) {
  const auto i(std::find(children_.begin(), children_.end(), view));
  if (i == children_.end())
    return;

  view->parent_ = nullptr;
  children_.erase(i);
  PlatformRemoveChildView(view);

  Layout();
}

void View::Layout() {
  // Do nothing in base.
}

}  // namespace nu
