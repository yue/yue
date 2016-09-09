// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#include "nativeui/win/base_view.h"

namespace nu {

namespace {

class ContainerView : public BaseView {
 public:
  explicit ContainerView(Container* delegate)
      : BaseView(ControlType::Container), delegate_(delegate) {}
  ~ContainerView() override {}

  void SizeAllocate(const Rect& size_allocation) override {
    // Container doesn't draw anything, so only do layout.
    set_size_allocation(size_allocation);
    delegate_->Layout();
  }

  void OnMouseMove(UINT flags, const Point& point) override {
    // Find the view that has the mouse.
    View* hover_view = FindChildFromPoint(point);

    // Emit mouse enter/leave events
    if (hover_view_ != hover_view) {
      if (hover_view_)
        hover_view_->view()->OnMouseLeave();
      hover_view_ = hover_view;
      if (hover_view_)
        hover_view_->view()->OnMouseEnter();
    }
    // Emit mouse move events.
    if (hover_view_) {
      Point child_point = point;
      child_point -= hover_view_->GetPixelBounds().OffsetFromOrigin();
      hover_view_->view()->OnMouseMove(flags, child_point);
    }
  }

  void OnMouseLeave() override {
    if (hover_view_) {
      hover_view_->view()->OnMouseLeave();
      hover_view_ = nullptr;
    }
  }

  void OnMouseClick(UINT message, UINT flags, const Point& point) override {
    View* child = FindChildFromPoint(point);
    if (child) {
      Point child_point = point;
      child_point -= child->GetPixelBounds().OffsetFromOrigin();
      child->view()->OnMouseClick(message, flags, child_point);
    }
  }

  void Draw(PainterWin* painter, const Rect& dirty) override {
    // Iterate children for drawing.
    for (int i = 0; i < delegate_->child_count(); ++i) {
      View* child = delegate_->child_at(i);
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

  void SetParent(BaseView* parent) override {
    BaseView::SetParent(parent);
    RefreshParentTree();
  }

  void BecomeContentView(WindowImpl* parent) override {
    BaseView::BecomeContentView(parent);
    RefreshParentTree();
  }

 private:
  void RefreshParentTree() {
    for (int i = 0; i < delegate_->child_count(); ++i)
      delegate_->child_at(i)->view()->SetParent(this);
  }

  View* FindChildFromPoint(const Point& point) {
    for (int i = 0; i < delegate_->child_count(); ++i) {
      View* child = delegate_->child_at(i);
      if (!child->IsVisible())
        continue;
      Rect child_bounds = child->GetPixelBounds();
      if (child_bounds.Contains(point))
        return child;
    }
    return nullptr;
  }

  Container* delegate_;

  // The View in which mouse hovers.
  View* hover_view_ = nullptr;
};

}  // namespace

void Container::PlatformInit() {
  TakeOverView(new ContainerView(this));
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
