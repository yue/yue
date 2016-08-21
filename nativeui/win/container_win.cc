// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#include "nativeui/win/virtual_view.h"

namespace nu {

namespace {

class ContainerView : public VirtualView {
 public:
  explicit ContainerView(Container* container) : container_(container) {}
  ~ContainerView() override {}

  void SetPixelBounds(const gfx::Rect& pixel_bounds) override {
    VirtualView::SetPixelBounds(pixel_bounds);
    container_->Layout();
  }

  void SetParent(BaseView* parent) override {
    VirtualView::SetParent(parent);
    // Refresh all children when parent window changes.
    for (int i = 0; i < container_->child_count(); ++i)
      container_->child_at(i)->view()->SetParent(this);
  }

 private:
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
