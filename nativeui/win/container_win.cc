// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#include "nativeui/win/base_view.h"

namespace nu {

namespace {

class ContainerView : public BaseView {
 public:
  explicit ContainerView(Container* container)
      : BaseView(true), container_(container) {}
  ~ContainerView() override {}

  void SetPixelBounds(const Rect& pixel_bounds) override {
    BaseView::SetPixelBounds(pixel_bounds);
    container_->Layout();
  }

  void Draw(Gdiplus::Graphics* context, const Rect& dirty) override {
    // Calculate the dirty rect for each child.
    for (int i = 0; i < container_->child_count(); ++i) {
      View* child = container_->child_at(i);
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
    for (int i = 0; i < container_->child_count(); ++i)
      container_->child_at(i)->view()->SetParent(this);
  }

  Container* container_;
};

}  // namespace

void Container::PlatformInit() {
  set_view(new ContainerView(this));
}

void Container::PlatformAddChildView(View* child) {
  child->view()->SetParent(view());
}

void Container::PlatformRemoveChildView(View* child) {
  child->view()->SetParent(nullptr);
}

}  // namespace nu
