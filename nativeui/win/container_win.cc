// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/container_win.h"

#include "base/stl_util.h"
#include "nativeui/gfx/win/painter_win.h"

namespace nu {

ContainerImpl::ContainerImpl(Delegate* delegate, ControlType type)
    : ViewImpl(type), delegate_(delegate) {}

void ContainerImpl::SizeAllocate(const Rect& size_allocation) {
  ViewImpl::SizeAllocate(size_allocation);
  if (!size_allocation.size().IsEmpty())
    delegate_->Layout();
}

void ContainerImpl::SetParent(ViewImpl* parent) {
  ViewImpl::SetParent(parent);
  RefreshParentTree();
}

void ContainerImpl::BecomeContentView(WindowImpl* parent) {
  ViewImpl::BecomeContentView(parent);
  RefreshParentTree();
}

void ContainerImpl::SetVisible(bool visible) {
  ViewImpl::SetVisible(visible);
  const auto& children = delegate_->GetChildren();
  for (ViewImpl* child : children)
    child->SetVisible(visible);
}

void ContainerImpl::Draw(PainterWin* painter, const Rect& dirty) {
  ViewImpl::Draw(painter, dirty);
  delegate_->OnDraw(painter, dirty);
  for (ViewImpl* child : delegate_->GetChildren())
    DrawChild(child, painter, dirty);
}

void ContainerImpl::OnMouseMove(UINT flags, const Point& point) {
  // Find the view that has the mouse.
  ViewImpl* hover_view = FindChildFromPoint(point);

  // Emit mouse enter/leave events
  if (hover_view_ != hover_view) {
    if (hover_view_ &&
        base::ContainsValue(delegate_->GetChildren(), hover_view_))
      hover_view_->OnMouseLeave();
    hover_view_ = hover_view;
    if (hover_view_)
      hover_view_->OnMouseEnter();
  }
  // Emit mouse move events.
  if (hover_view_)
    hover_view_->OnMouseMove(flags, point);
}

void ContainerImpl::OnMouseLeave() {
  if (hover_view_) {
    if (base::ContainsValue(delegate_->GetChildren(), hover_view_))
      hover_view_->OnMouseLeave();
    hover_view_ = nullptr;
  }
}

bool ContainerImpl::OnMouseWheel(bool vertical, UINT flags, int delta,
                                 const Point& point) {
  ViewImpl* child = FindChildFromPoint(point);
  if (child)
    return child->OnMouseWheel(vertical, flags, delta, point);
  return false;
}

bool ContainerImpl::OnMouseClick(UINT message, UINT flags, const Point& point) {
  ViewImpl* child = FindChildFromPoint(point);
  if (child)
    return child->OnMouseClick(message, flags, point);
  return false;
}

void ContainerImpl::DrawChild(ViewImpl* child, PainterWin* painter,
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

void ContainerImpl::RefreshParentTree() {
  const auto& children = delegate_->GetChildren();
  for (ViewImpl* child : children)
    child->SetParent(this);
}

ViewImpl* ContainerImpl::FindChildFromPoint(const Point& point) {
  const auto& children = delegate_->GetChildren();
  for (ViewImpl* child : children) {
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

class ContainerAdapter : public ContainerImpl,
                         public ContainerImpl::Delegate {
 public:
  explicit ContainerAdapter(Container* container)
      : ContainerImpl(this, ControlType::Container), container_(container) {}

  // ContainerImpl::Delegate:
  void Layout() override {
    container_->BoundsChanged();
  }

  std::vector<ViewImpl*> GetChildren() override {
    std::vector<ViewImpl*> views(container_->ChildCount());
    for (int i = 0; i < container_->ChildCount(); ++i)
      views[i] = container_->ChildAt(i)->GetNative();
    return views;
  }

  void OnDraw(PainterWin* painter, const Rect& dirty) override {
    float scale_factor = container_->GetNative()->scale_factor();
    container_->on_draw.Emit(container_, static_cast<Painter*>(painter),
                             ScaleRect(RectF(dirty), 1.0f / scale_factor));
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
  child->GetNative()->SetParent(GetNative());
}

void Container::PlatformRemoveChildView(View* child) {
  child->GetNative()->SetParent(nullptr);
}

}  // namespace nu
