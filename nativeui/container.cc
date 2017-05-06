// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#include <algorithm>
#include <limits>

#include "base/logging.h"
#include "third_party/yoga/yoga/Yoga.h"

namespace nu {

namespace {

// Whether a Container is a root CSS node.
inline bool IsRootYGNode(Container* view) {
  return !YGNodeGetParent(view->node()) || !view->GetParent();
}

// Get bounds from the CSS node.
inline RectF GetYGNodeBounds(YGNodeRef node) {
  return RectF(YGNodeLayoutGetLeft(node), YGNodeLayoutGetTop(node),
               YGNodeLayoutGetWidth(node), YGNodeLayoutGetHeight(node));
}

}  // namespace

// static
const char Container::kClassName[] = "Container";

Container::Container() {
  PlatformInit();
}

Container::Container(const char* an_empty_constructor) {
}

Container::~Container() {
  PlatformDestroy();
}

const char* Container::GetClassName() const {
  return kClassName;
}

void Container::Layout() {
  // For child CSS node, tell parent to do the layout.
  if (!IsRootYGNode(this)) {
    static_cast<Container*>(GetParent())->Layout();
    return;
  }

  // So this is a root CSS node, calculate the layout and set bounds.
  SizeF size(GetBounds().size());
  YGNodeCalculateLayout(node(), size.width(), size.height(), YGDirectionLTR);
  SetChildBoundsFromCSS();
}

void Container::OnSizeChanged() {
  View::OnSizeChanged();
  if (IsRootYGNode(this))
    Layout();
  else
    SetChildBoundsFromCSS();
}

SizeF Container::GetPreferredSize() const {
  float nan = std::numeric_limits<float>::quiet_NaN();
  YGNodeCalculateLayout(node(), nan, nan, YGDirectionLTR);
  return SizeF(YGNodeLayoutGetWidth(node()), YGNodeLayoutGetHeight(node()));
}

float Container::GetPreferredHeightForWidth(float width) const {
  float nan = std::numeric_limits<float>::quiet_NaN();
  YGNodeCalculateLayout(node(), width, nan, YGDirectionLTR);
  return YGNodeLayoutGetHeight(node());
}

float Container::GetPreferredWidthForHeight(float height) const {
  float nan = std::numeric_limits<float>::quiet_NaN();
  YGNodeCalculateLayout(node(), nan, height, YGDirectionLTR);
  return YGNodeLayoutGetWidth(node());
}

void Container::AddChildView(View* view) {
  DCHECK(view);
  if (view->GetParent() == this)
    return;
  AddChildViewAt(view, ChildCount());
}

void Container::AddChildViewAt(View* view, int index) {
  DCHECK(view);
  if (view == this || index < 0 || index > ChildCount())
    return;

  if (view->GetParent()) {
    LOG(ERROR) << "The view already has a parent.";
    return;
  }

  YGNodeInsertChild(node(), view->node(), index);
  view->SetParent(this);

  children_.insert(children_.begin() + index, view);
  PlatformAddChildView(view);

  DCHECK_EQ(static_cast<int>(YGNodeGetChildCount(node())), ChildCount());

  Layout();
}

void Container::RemoveChildView(View* view) {
  const auto i(std::find(children_.begin(), children_.end(), view));
  if (i == children_.end())
    return;

  view->SetParent(nullptr);
  YGNodeRemoveChild(node(), view->node());

  PlatformRemoveChildView(view);
  children_.erase(i);

  DCHECK_EQ(static_cast<int>(YGNodeGetChildCount(node())), ChildCount());

  Layout();
}

void Container::SetChildBoundsFromCSS() {
  for (int i = 0; i < ChildCount(); ++i) {
    View* child = ChildAt(i);
    if (child->IsVisible())
      child->SetBounds(GetYGNodeBounds(child->node()));
  }
}

}  // namespace nu
