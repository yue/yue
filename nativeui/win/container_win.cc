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
  if (!size_allocation.size().IsEmpty())
    delegate_->Layout();
}

void ContainerView::OnMouseMove(UINT flags, const Point& point) {
  // Find the view that has the mouse.
  BaseView* hover_view = FindChildFromPoint(point);

  // Emit mouse enter/leave events
  if (hover_view_ != hover_view) {
    if (hover_view_ &&
        ContainsValue(delegate_->GetChildren(), hover_view_))
      hover_view_->OnMouseLeave();
    hover_view_ = hover_view;
    if (hover_view_)
      hover_view_->OnMouseEnter();
  }
  // Emit mouse move events.
  if (hover_view_)
    hover_view_->OnMouseMove(flags, point);
}

void ContainerView::OnMouseLeave() {
  if (hover_view_) {
    if (ContainsValue(delegate_->GetChildren(), hover_view_))
      hover_view_->OnMouseLeave();
    hover_view_ = nullptr;
  }
}

bool ContainerView::OnMouseWheel(bool vertical, UINT flags, int delta,
                                 const Point& point) {
  BaseView* child = FindChildFromPoint(point);
  if (child)
    return child->OnMouseWheel(vertical, flags, delta, point);
  return false;
}

bool ContainerView::OnMouseClick(UINT message, UINT flags, const Point& point) {
  BaseView* child = FindChildFromPoint(point);
  if (child)
    return child->OnMouseClick(message, flags, point);
  return false;
}

void ContainerView::Draw(PainterWin* painter, const Rect& dirty) {
  BaseView::Draw(painter, dirty);
  for (BaseView* child : delegate_->GetChildren())
    DrawChild(child, painter, dirty);
}

void ContainerView::SetParent(BaseView* parent) {
  BaseView::SetParent(parent);
  RefreshParentTree();
}

void ContainerView::BecomeContentView(WindowImpl* parent) {
  BaseView::BecomeContentView(parent);
  RefreshParentTree();
}

void ContainerView::DrawChild(BaseView* child, PainterWin* painter,
                              const Rect& dirty) {
  if (!child->is_visible())
    return;

  // Caculate the dirty rect for child.
  Rect child_dirty(child->GetClippedRect() -
                   size_allocation().OffsetFromOrigin());
  child_dirty.Intersect(dirty);
  if (child_dirty.IsEmpty())
    return;

  // Move the painting origin for child.
  Vector2d child_origin = child->size_allocation().OffsetFromOrigin() -
                          size_allocation().OffsetFromOrigin();
  painter->Save();
  painter->TranslatePixel(child_origin);
  child->Draw(painter, child_dirty - child_origin);
  painter->Restore();
}

void ContainerView::RefreshParentTree() {
  const auto& children = delegate_->GetChildren();
  for (BaseView* child : children)
    child->SetParent(this);
}

BaseView* ContainerView::FindChildFromPoint(const Point& point) {
  const auto& children = delegate_->GetChildren();
  for (BaseView* child : children) {
    if (!child->is_visible())
      continue;
    Rect child_rect = child->GetClippedRect();
    if (child_rect.Contains(point))
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
    container_->BoundsChanged();
  }

  std::vector<BaseView*> GetChildren() override {
    std::vector<BaseView*> views(container_->child_count());
    for (int i = 0; i < container_->child_count(); ++i)
      views[i] = container_->child_at(i)->view();
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
