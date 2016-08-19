// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

namespace nu {

View::View() : view_(nullptr) {
}

View::~View() {
  gtk_widget_destroy(view_);
}

void View::SetBounds(const gfx::Rect& bounds) {
  gtk_widget_set_size_request(view_, bounds.width(), bounds.height());
}

gfx::Rect View::GetBounds() {
  GtkRequisition requisition;
  gtk_widget_size_request(view_, &requisition);
  return gfx::Rect(0, 0, requisition.width, requisition.height);
}

}  // namespace nu
