// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/base_view.h"

namespace nu {

void BaseView::SetPixelBounds(const gfx::Rect& bounds) {
  bounds_ = bounds;

  // Refresh the origin to parent HWND.
  if (parent()) {
    gfx::Point po = parent()->GetWindowPixelOrigin();
    window_origin_.set_x(bounds.x() + po.x());
    window_origin_.set_y(bounds.y() + po.y());
  } else if (window_) {
    DCHECK(is_content_view_);
    window_origin_ = bounds.origin();
  } else {
    DCHECK(!is_content_view_);
    window_origin_ = gfx::Point();
  }
}

gfx::Rect BaseView::GetPixelBounds() {
  return bounds_;
}

void BaseView::SetBounds(const gfx::Rect& bounds) {
  SetPixelBounds(ScaleToEnclosingRect(bounds, scale_factor()));
}

gfx::Rect BaseView::GetBounds() {
  return ScaleToEnclosingRect(GetPixelBounds(), 1.0f / scale_factor());
}

gfx::Point BaseView::GetWindowPixelOrigin() {
  return window_origin_;
}

void BaseView::SetParent(BaseView* parent) {
  float old_scale_factor = scale_factor();

  is_content_view_ = false;
  parent_ = parent;
  window_ = parent ? parent->window_ : nullptr;

  // Scale the bounds after moving to a new parent.
  if (old_scale_factor != scale_factor())
    bounds_ = ScaleToEnclosingRect(bounds_, scale_factor() / old_scale_factor);
}

void BaseView::BecomeContentView(WindowImpl* parent) {
  float old_scale_factor = scale_factor();

  is_content_view_ = true;
  parent_ = nullptr;
  window_ = parent;

  // Scale the bounds after moving to a new parent.
  if (old_scale_factor != scale_factor())
    bounds_ = ScaleToEnclosingRect(bounds_, scale_factor() / old_scale_factor);
}

}  // namespace nu
