// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/container_win.h"

#include "base/stl_util.h"

namespace nu {

ContainerView::ContainerView(Delegate* delegate, ControlType type)
    : BaseView(type), delegate_(delegate) {}

void ContainerView::SizeAllocate(const Rect& size_allocation) {
  BaseView::SizeAllocate(size_allocation);
  delegate_->Layout();
}

void ContainerView::OnMouseMove(UINT flags, const Point& point) {
  // Find the view that has the mouse.
  View* hover_view = FindChildFromPoint(point);

  // Emit mouse enter/leave events
  if (hover_view_ != hover_view) {
    if (hover_view_ &&
        ContainsValue(delegate_->GetChildren(), hover_view_))
      hover_view_->view()->OnMouseLeave();
    hover_view_ = hover_view;
    if (hover_view_)
      hover_view_->view()->OnMouseEnter();
  }
  // Emit mouse move events.
  if (hover_view_)
    hover_view_->view()->OnMouseMove(flags, point);
}

void ContainerView::OnMouseLeave() {
  if (hover_view_) {
    if (ContainsValue(delegate_->GetChildren(), hover_view_))
      hover_view_->view()->OnMouseLeave();
    hover_view_ = nullptr;
  }
}

bool ContainerView::OnMouseWheel(bool vertical, UINT flags, int delta,
                                 const Point& point) {
  View* child = FindChildFromPoint(point);
  if (child)
    return child->view()->OnMouseWheel(vertical, flags, delta, point);
  return false;
}

void ContainerView::OnMouseClick(UINT message, UINT flags, const Point& point) {
  View* child = FindChildFromPoint(point);
  if (child)
    child->view()->OnMouseClick(message, flags, point);
}

void ContainerView::Draw(PainterWin* painter, const Rect& dirty) {
  // Iterate children for drawing.
  const auto& children = delegate_->GetChildren();
  for (View* child : children) {
    if (!child->IsVisible())
      continue;
    Rect child_bounds = child->GetPixelBounds();
    if (child_bounds.Intersects(dirty)) {
      // Caculate the dirty rect for child.
      Rect child_dirty(dirty);
      child_dirty.Intersect(child_bounds);
      child_dirty -= child_bounds.OffsetFromOrigin();

      // Move the painting origin for child.
      painter->Save();
      painter->Translate(child_bounds.OffsetFromOrigin());
      child->view()->Draw(painter, child_dirty);
      painter->Restore();
    }
  }
}

void ContainerView::SetParent(BaseView* parent) {
  BaseView::SetParent(parent);
  RefreshParentTree();
}

void ContainerView::BecomeContentView(WindowImpl* parent) {
  BaseView::BecomeContentView(parent);
  RefreshParentTree();
}

void ContainerView::RefreshParentTree() {
  const auto& children = delegate_->GetChildren();
  for (View* child : children)
    child->view()->SetParent(this);
}

View* ContainerView::FindChildFromPoint(const Point& point) {
  const auto& children = delegate_->GetChildren();
  for (View* child : children) {
    if (!child->IsVisible())
      continue;
    Rect child_bounds = child->view()->size_allocation();
    if (child_bounds.Contains(point))
      return child;
  }
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// Adapter from Container to Container::Delegate.

namespace {

class ContainerAdapter : public ContainerView,
                         public ContainerView::Delegate {
 public:
  explicit ContainerAdapter(Container* container)
      : ContainerView(this, ControlType::Container), container_(container) {}

  // ContainerView::Delegate:
  void Layout() override {
    container_->Layout();
  }

  std::vector<View*> GetChildren() override {
    std::vector<View*> views(container_->child_count());
    for (int i = 0; i < container_->child_count(); ++i)
      views[i] = container_->child_at(i);
    return views;
  }

 private:
  Container* container_;
};

}  // namespace

///////////////////////////////////////////////////////////////////////////////
// Public Container API implementation.

void Container::PlatformInit() {
  TakeOverView(new ContainerAdapter(this));
}

void Container::PlatformDestroy() {
}

void Container::PlatformAddChildView(View* child) {
  child->view()->SetParent(view());
}

void Container::PlatformRemoveChildView(View* child) {
  child->view()->SetParent(nullptr);
}

}  // namespace nu
