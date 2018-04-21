// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/container_win.h"

#include "base/stl_util.h"
#include "nativeui/events/win/event_win.h"
#include "nativeui/gfx/win/painter_win.h"

namespace nu {

ContainerImpl::ContainerImpl(View* delegate, Adapter* adapter, ControlType type)
    : ViewImpl(type, delegate), adapter_(adapter) {}

void ContainerImpl::SizeAllocate(const Rect& size_allocation) {
  ViewImpl::SizeAllocate(size_allocation);
  if (!size_allocation.size().IsEmpty())
    adapter_->Layout();
}

UINT ContainerImpl::HitTest(const Point& point) const {
  ViewImpl* child = FindChildFromPoint(point);
  if (child)
    return child->HitTest(point);
  return ViewImpl::HitTest(point);
}

void ContainerImpl::SetParent(ViewImpl* parent) {
  ViewImpl::SetParent(parent);
  RefreshParentTree();
}

void ContainerImpl::BecomeContentView(WindowImpl* parent) {
  ViewImpl::BecomeContentView(parent);
  RefreshParentTree();
}

void ContainerImpl::VisibilityChanged() {
  ViewImpl::VisibilityChanged();
  adapter_->ForEach([=](ViewImpl* child) {
    child->VisibilityChanged();
    return true;
  });
}

void ContainerImpl::Draw(PainterWin* painter, const Rect& dirty) {
  if (!is_visible())
    return;
  ViewImpl::Draw(painter, dirty);
  adapter_->OnDraw(painter, dirty);
  adapter_->ForEach([&](ViewImpl* child) {
    DrawChild(child, painter, dirty);
    return true;
  });
}

void ContainerImpl::OnMouseMove(NativeEvent event) {
  // Find the view that has the mouse.
  Point point(event->l_param);
  ViewImpl* hover_view = FindChildFromPoint(point);

  // Emit mouse enter/leave events
  if (hover_view_ != hover_view) {
    if (hover_view_ && adapter_->HasChild(hover_view_))
      hover_view_->OnMouseLeave(event);
    hover_view_ = hover_view;
    if (hover_view_)
      hover_view_->OnMouseEnter(event);
  }
  // Emit mouse move events.
  if (hover_view_)
    hover_view_->OnMouseMove(event);

  ViewImpl::OnMouseMove(event);
}

void ContainerImpl::OnMouseLeave(NativeEvent event) {
  if (hover_view_) {
    if (adapter_->HasChild(hover_view_))
      hover_view_->OnMouseLeave(event);
    hover_view_ = nullptr;
  }
  ViewImpl::OnMouseLeave(event);
}

bool ContainerImpl::OnMouseWheel(NativeEvent event) {
  ViewImpl* child = FindChildFromPoint(Point(event->l_param));
  if (child)
    return child->OnMouseWheel(event);
  return false;
}

bool ContainerImpl::OnMouseClick(NativeEvent event) {
  ViewImpl* child = FindChildFromPoint(Point(event->l_param));
  if (child && child->OnMouseClick(event))
    return true;
  return ViewImpl::OnMouseClick(event);
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
  adapter_->ForEach([this](ViewImpl* child) {
    child->SetParent(this);
    return true;
  });
}

ViewImpl* ContainerImpl::FindChildFromPoint(const Point& point) const {
  ViewImpl* result = nullptr;
  adapter_->ForEach([&](ViewImpl* child) {
    if (!child->is_visible())
      return true;
    Rect child_rect = child->GetClippedRect();
    if (child_rect.Contains(point)) {
      result = child;
      return false;
    }
    return true;
  });
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// Adapter from Container to Container::Adapter.

namespace {

class ContainerAdapter : public ContainerImpl,
                         public ContainerImpl::Adapter {
 public:
  explicit ContainerAdapter(Container* container)
      : ContainerImpl(container, this), container_(container) {}

  // ContainerImpl::Adapter:
  void Layout() override {
    container_->SetChildBoundsFromCSS();
  }

  void ForEach(const std::function<bool(ViewImpl*)>& callback,
               bool reverse) override {
    for (int i = reverse ? container_->ChildCount() - 1 : 0;
         reverse ? (i >= 0) : (i < container_->ChildCount());
         reverse ? --i : ++i) {
      if (!callback(container_->ChildAt(i)->GetNative()))
        break;
    }
  }

  bool HasChild(ViewImpl* child) override {
    for (int i = 0; i < container_->ChildCount(); ++i) {
      if (child == container_->ChildAt(i)->GetNative())
        return true;
    }
    return false;
  }

  void OnDraw(PainterWin* painter, const Rect& dirty) override {
    if (container_->on_draw.IsEmpty())
      return;
    painter->Save();
    painter->ClipRectPixel(Rect(size_allocation().size()));
    float scale_factor = container_->GetNative()->scale_factor();
    container_->on_draw.Emit(container_, static_cast<Painter*>(painter),
                             ScaleRect(RectF(dirty), 1.0f / scale_factor));
    painter->Restore();
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
