// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#include <algorithm>

#include "base/logging.h"
#include "nativeui/layout/fill_layout.h"
#include "third_party/css-layout/CSSLayout/CSSLayout.h"

namespace nu {

namespace {

// Whether a view is a Container.
inline bool IsContainer(View* view) {
  return view->GetClassName() == Container::kClassName;
}

// Whether a Container is a root CSS node.
inline bool IsRootCSSNode(Container* view) {
  return !view->parent() || !IsContainer(view->parent()) ||
         !static_cast<Container*>(view->parent())->GetLayoutManager();
}

// Get bounds from the CSS node.
inline Rect GetCSSNodeBounds(CSSNodeRef node) {
  return Rect(CSSNodeLayoutGetLeft(node), CSSNodeLayoutGetTop(node),
              CSSNodeLayoutGetWidth(node), CSSNodeLayoutGetHeight(node));
}

}  // namespace

// static
const char Container::kClassName[] = "Container";

Container::Container() {
  PlatformInit();
}

Container::~Container() {
  PlatformDestroy();
}

bool Container::UpdatePreferredSize() {
  if (!layout_manager_ ||
      SetPixelPreferredSize(layout_manager_->GetPixelPreferredSize(this)))
    Layout();

  // The layout of children is managed by Container.
  return false;
}

const char* Container::GetClassName() const {
  return kClassName;
}

void Container::SetLayoutManager(LayoutManager* layout_manager) {
  if (!layout_manager)
    return;

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

  // Do normal layout if there is a layout manager.
  if (layout_manager_) {
    layout_manager_->Layout(this);
    return;
  }

  // For child CSS node, tell parent to do the layout.
  if (!IsRootCSSNode(this)) {
    static_cast<Container*>(parent())->Layout();
    return;
  }

  // So this is a root CSS node, calculate the layout and set bounds.
  Size size(GetBounds().size());
  CSSNodeCalculateLayout(node(), size.width(), size.height(), CSSDirectionLTR);
  SetChildBoundsFromCSS();
}

void Container::BoundsChanged() {
  // For normal layouts and root CSS node, do a complete layout.
  if (layout_manager_ || IsRootCSSNode(this)) {
    Layout();
    return;
  }

  // The layout has been calculated by parent node, just set bounds.
  SetChildBoundsFromCSS();
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

  view->set_parent(this);
  if (layout_manager_)
    layout_manager_->ViewAdded(this, view);
  CSSNodeInsertChild(node(), view->node(), index);

  children_.insert(children_.begin() + index, view);
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
  if (layout_manager_)
    layout_manager_->ViewRemoved(this, view);
  CSSNodeRemoveChild(node(), view->node());

  PlatformRemoveChildView(view);
  children_.erase(i);

  DCHECK_EQ(static_cast<int>(CSSNodeChildCount(node())), child_count());

  if (UpdatePreferredSize())
    Layout();
}

void Container::SetChildBoundsFromCSS() {
  for (int i = 0; i < child_count(); ++i) {
    View* child = child_at(i);
    if (child->IsVisible())
      child->SetBounds(GetCSSNodeBounds(child->node()));
  }
}

}  // namespace nu
