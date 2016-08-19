// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#include <algorithm>

#include "base/logging.h"
#include "nativeui/layout/fill_layout.h"

namespace nu {

Container::Container() : layout_manager_(new FillLayout) {
  PlatformInit();
}

Container::~Container() {
}

void Container::SetLayoutManager(LayoutManager* layout_manager) {
  layout_manager_ = layout_manager;
  Layout();
}

LayoutManager* Container::GetLayoutManager() const {
  return layout_manager_.get();
}

void Container::Layout() {
  DCHECK(layout_manager_.get());
  layout_manager_->Layout(this);
}

void Container::AddChildView(View* view) {
  if (view->parent() == this)
    return;
  AddChildViewAt(view, child_count());
}

void Container::AddChildViewAt(View* view, int index) {
  if (view == this || index < 0 || index > child_count())
    return;

  // TODO(zcbenz): support moving views.
  if (view->parent()) {
    LOG(ERROR) << "The view already has a parent.";
    return;
  }

  children_.insert(children_.begin() + index, view);
  view->set_parent(this);
  PlatformAddChildView(view);

  Layout();
}

void Container::RemoveChildView(View* view) {
  const auto i(std::find(children_.begin(), children_.end(), view));
  if (i == children_.end())
    return;

  PlatformRemoveChildView(view);
  view->set_parent(nullptr);
  children_.erase(i);

  Layout();
}

}  // namespace nu
