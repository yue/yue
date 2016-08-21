// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#include "nativeui/win/subwin_view.h"

namespace nu {

void Container::PlatformInit() {
  set_view(new SubwinView());
}

void Container::PlatformAddChildView(View* child) {
  SetParent(child->view()->hwnd(), view()->hwnd());
}

void Container::PlatformRemoveChildView(View* child) {
  SetParent(child->view()->hwnd(), NULL);
}

}  // namespace nu
