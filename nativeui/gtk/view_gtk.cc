// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#include "nativeui/container.h"

namespace nu {

View::View() : view_(nullptr) {
}

View::~View() {
  gtk_widget_destroy(view_);
}

void View::SetBounds(const gfx::Rect& bounds) {
  GdkRectangle rect = { bounds.x(), bounds.y(),
                        bounds.width(), bounds.height() };
  if (parent()) {
    // The size allocation is relative to the window instead of parent.
    rect.x += parent()->GetBounds().x();
    rect.y += parent()->GetBounds().y();
  }
  gtk_widget_size_allocate(view_, &rect);
}

gfx::Rect View::GetBounds() {
  GdkRectangle rect;
  gtk_widget_get_allocation(view_, &rect);
  return gfx::Rect(rect.x, rect.y, rect.width, rect.height);
}

}  // namespace nu
