// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/virtual_view.h"

namespace nu {

VirtualView::VirtualView() : BaseView(true) {
  Init(NULL, 1.0f);
}

void VirtualView::SetPixelBounds(const gfx::Rect& pixel_bounds) {
  bounds_ = pixel_bounds;
}

gfx::Rect VirtualView::GetPixelBounds() {
  return bounds_;
}

void VirtualView::SetParent(BaseView* parent) {
  BaseView::SetParent(parent);
  if (!parent) {
    Init(NULL, scale_factor());
    return;
  }

  // Scale the bounds after moving to a new parent.
  if (parent->scale_factor() != scale_factor()) {
    bounds_ = ScaleToEnclosingRect(bounds_,
                                   parent->scale_factor() / scale_factor());
  }

  Init(parent->hwnd(), parent->scale_factor());
}

}  // namespace nu
