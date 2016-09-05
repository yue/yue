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
      : BaseView(true), delegate_(delegate) {}
  ~ContainerView() override {}

  void SetPixelBounds(const Rect& pixel_bounds) override {
    BaseView::SetPixelBounds(pixel_bounds);
    delegate_->Layout();
  }

  void OnMouseMove(const Point& point) override {
    // Find the view that has the mouse.
    View* hover_view = nullptr;
    for (int i = 0; i < delegate_->child_count(); ++i) {
      View* child = delegate_->child_at(i);
      if (!child->IsVisible())
        continue;
      Rect child_bounds = child->GetPixelBounds();
      if (child_bounds.Contains(point)) {
        hover_view = child;
        break;
      }
    }

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
      hover_view_->view()->OnMouseMove(child_point);
    }
  }

  void OnMouseLeave() override {
    if (hover_view_) {
      hover_view_->view()->OnMouseLeave();
      hover_view_ = nullptr;
    }
  }

  void Draw(Gdiplus::Graphics* context, const Rect& dirty) override {
    // Calculate the dirty rect for each child.
    for (int i = 0; i < delegate_->child_count(); ++i) {
      View* child = delegate_->child_at(i);
      if (!child->IsVisible())
        continue;
      Rect child_bounds = child->GetPixelBounds();
      if (child_bounds.Intersects(dirty)) {
        Rect child_dirty(dirty);
        child_dirty.Intersect(child_bounds);
        child_dirty -= child_bounds.OffsetFromOrigin();
        child->view()->Draw(context, child_dirty);
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

  Container* delegate_;

  // The View in which mouse hovers.
  View* hover_view_ = nullptr;
};

}  // namespace

void Container::PlatformInit() {
  TakeOverView(new ContainerView(this));
}

void Container::PlatformAddChildView(View* child) {
  child->view()->SetParent(view());
}

void Container::PlatformRemoveChildView(View* child) {
  child->view()->SetParent(nullptr);
}

}  // namespace nu
