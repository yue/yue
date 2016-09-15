// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#include <algorithm>
#include <limits>

#include "base/logging.h"
#include "third_party/css-layout/CSSLayout/CSSLayout.h"
#include "third_party/css-layout/CSSLayout/CSSLayout-internal.h"

extern "C"
bool layoutNodeInternal(const CSSNodeRef node,
                        const float availableWidth,
                        const float availableHeight,
                        const CSSDirection parentDirection,
                        const CSSMeasureMode widthMeasureMode,
                        const CSSMeasureMode heightMeasureMode,
                        const bool performLayout,
                        const char *reason);

namespace nu {

namespace {

// Whether a view is a Container.
inline bool IsContainer(View* view) {
  return view->GetClassName() == Container::kClassName;
}

// Whether a Container is a root CSS node.
inline bool IsRootCSSNode(Container* view) {
  return !view->parent() || !IsContainer(view->parent());
}

// Get bounds from the CSS node.
inline RectF GetCSSNodeBounds(CSSNodeRef node) {
  return RectF(CSSNodeLayoutGetLeft(node), CSSNodeLayoutGetTop(node),
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

const char* Container::GetClassName() const {
  return kClassName;
}

void Container::Layout() {
  // No need to layout when container is not initialized.
  if (GetPixelBounds().IsEmpty())
    return;

  // For child CSS node, tell parent to do the layout.
  if (!IsRootCSSNode(this)) {
    static_cast<Container*>(parent())->Layout();
    return;
  }

  // So this is a root CSS node, calculate the layout and set bounds.
  SizeF size(GetBounds().size());
  CSSNodeCalculateLayout(node(), size.width(), size.height(), CSSDirectionLTR);
  SetChildBoundsFromCSS();
}

void Container::BoundsChanged() {
  if (IsRootCSSNode(this))
    Layout();
  else
    SetChildBoundsFromCSS();
}

SizeF Container::GetPreferredSize() const {
  float float_max = std::numeric_limits<float>::max();
  layoutNodeInternal(node(), float_max, float_max, CSSDirectionLTR,
                     CSSMeasureModeAtMost, CSSMeasureModeAtMost,
                     false, "measure");
  return SizeF(node()->layout.measuredDimensions[CSSDimensionWidth],
               node()->layout.measuredDimensions[CSSDimensionHeight]);
}

float Container::GetPreferredHeightForWidth(float width) const {
  layoutNodeInternal(node(), width, std::numeric_limits<float>::max(),
                     CSSDirectionLTR, CSSMeasureModeExactly,
                     CSSMeasureModeAtMost, false, "measure");
  return node()->layout.measuredDimensions[CSSDimensionHeight];
}

float Container::GetPreferredWidthForHeight(float height) const {
  layoutNodeInternal(node(), std::numeric_limits<float>::max(), height,
                     CSSDirectionLTR, CSSMeasureModeAtMost,
                     CSSMeasureModeExactly, false, "measure");
  return node()->layout.measuredDimensions[CSSDimensionWidth];
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

  if (view->parent()) {
    LOG(ERROR) << "The view already has a parent.";
    return;
  }

  view->set_parent(this);
  CSSNodeInsertChild(node(), view->node(), index);

  children_.insert(children_.begin() + index, view);
  PlatformAddChildView(view);

  DCHECK_EQ(static_cast<int>(CSSNodeChildCount(node())), child_count());

  Layout();
}

void Container::RemoveChildView(View* view) {
  const auto i(std::find(children_.begin(), children_.end(), view));
  if (i == children_.end())
    return;

  view->set_parent(nullptr);
  CSSNodeRemoveChild(node(), view->node());

  PlatformRemoveChildView(view);
  children_.erase(i);

  DCHECK_EQ(static_cast<int>(CSSNodeChildCount(node())), child_count());

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
