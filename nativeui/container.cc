// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#include <algorithm>

#include "base/logging.h"
#include "nativeui/layout/fill_layout.h"
#include "third_party/css-layout/CSSLayout/CSSLayout.h"

namespace nu {

Container::Container() : layout_manager_(new FillLayout) {
  PlatformInit();
}

Container::~Container() {
  PlatformDestroy();
}

bool Container::UpdatePreferredSize() {
  if (SetPixelPreferredSize(layout_manager_->GetPixelPreferredSize(this)))
    Layout();

  // The layout of children is managed by Container.
  return false;
}

void Container::SetLayoutManager(LayoutManager* layout_manager) {
  DCHECK(layout_manager);
  layout_manager_ = layout_manager;
  if (UpdatePreferredSize())
    Layout();
}

LayoutManager* Container::GetLayoutManager() const {
  return layout_manager_.get();
}

void Container::Layout() {
  // No need to layout when container is not initialized.
  if (GetPixelBounds().IsEmpty())
    return;

  DCHECK(layout_manager_.get());
  layout_manager_->Layout(this);
}

void Container::AddChildView(View* view) {
  DCHECK(view);
  if (view->parent() == this)
    return;
  AddChildViewAt(view, child_count());
}

void Container::AddChildViewAt(View* view, int index) {
  DCHECK(view);
  if (view == this || index < 0 || index > child_count())
    return;

  // TODO(zcbenz): support moving views.
  if (view->parent()) {
    LOG(ERROR) << "The view already has a parent.";
    return;
  }

  children_.insert(children_.begin() + index, view);
  view->set_parent(this);
  layout_manager_->ViewAdded(this, view);
  CSSNodeInsertChild(node(), view->node(), index);
  PlatformAddChildView(view);

  DCHECK_EQ(static_cast<int>(CSSNodeChildCount(node())), child_count());

  if (UpdatePreferredSize())
    Layout();
}

void Container::RemoveChildView(View* view) {
  const auto i(std::find(children_.begin(), children_.end(), view));
  if (i == children_.end())
    return;

  view->set_parent(nullptr);
  layout_manager_->ViewRemoved(this, view);
  CSSNodeRemoveChild(node(), view->node());
  PlatformRemoveChildView(view);
  children_.erase(i);

  DCHECK_EQ(static_cast<int>(CSSNodeChildCount(node())), child_count());

  if (UpdatePreferredSize())
    Layout();
}

}  // namespace nu
