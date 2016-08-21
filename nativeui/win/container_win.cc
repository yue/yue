// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#include "nativeui/win/subwin_view.h"

namespace nu {

namespace {

class ContainerView : public SubwinView {
 public:
  explicit ContainerView(Container* container) : container_(container) {}
  ~ContainerView() override {}

 protected:
  CR_BEGIN_MSG_MAP_EX(SubwinView, WindowImpl)
    CR_MSG_WM_SIZE(OnSize)
  CR_END_MSG_MAP()

  void OnSize(UINT param, const gfx::Size& size) {
    container_->Layout();
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
