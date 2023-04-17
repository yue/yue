// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "base/logging.h"
#include "third_party/yoga/Yoga.h"

namespace nu {

namespace {

// Whether a Container is a root CSS node.
inline bool IsRootYGNode(Container* view) {
  return !YGNodeGetParent(view->node()) || !view->IsContainer();
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
    dirty_ = true;
    static_cast<Container*>(GetParent())->Layout();
    // The parent may choose to not update this view because its size is not
    // changed, in that case we need to force updating here.
    // This usually happens after adding a child view, since the container does
    // not change its size.
    // TODO(zcbenz): Revisit the logic here, should have a cleaner way.
    if (dirty_)
      UpdateChildBounds();
    return;
  }

  UpdateChildBounds();
}

bool Container::IsContainer() const {
  return true;
}

// Mac uses this event for layout, while other platforms do it in their
// native subclasses (i.e. nu_container_size_allocate in GTK and
// ContainerImpl::SizeAllocate in Win32).
#if defined(OS_MAC)
void Container::OnSizeChanged() {
  View::OnSizeChanged();
  UpdateChildBounds();
}
#endif

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

void Container::AddChildView(scoped_refptr<View> view) {
  DCHECK(view);
  if (view->GetParent() == this)
    return;
  AddChildViewAt(std::move(view), ChildCount());
}

void Container::AddChildViewAt(scoped_refptr<View> view, int index) {
  DCHECK(view);
  if (view == this || index < 0 || index > ChildCount())
    return;

  if (view->GetParent()) {
    LOG(ERROR) << "The view already has a parent.";
    return;
  }

  YGNodeInsertChild(node(), view->node(), index);
  view->SetParent(this);

  PlatformAddChildView(view.get());
  children_.insert(children_.begin() + index, std::move(view));

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

void Container::UpdateChildBounds() {
  dirty_ = false;
  if (!IsVisibleInHierarchy())
    return;
  // For root CSS node, calculate the layout before setting bounds.
  if (IsRootYGNode(this)) {
    SizeF size = GetBounds().size();
    YGNodeCalculateLayout(node(), size.width(), size.height(), YGDirectionLTR);
  }
  for (int i = 0; i < ChildCount(); ++i) {
    View* child = ChildAt(i);
    if (child->IsVisibleInHierarchy())
      child->SetBounds(GetYGNodeBounds(child->node()));
  }
}

}  // namespace nu
