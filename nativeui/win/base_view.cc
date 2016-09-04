// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/base_view.h"

namespace nu {

void BaseView::SetPixelBounds(const Rect& bounds) {
  bounds_ = bounds;

  // Refresh the origin to parent HWND.
  window_origin_ = bounds.origin();
  if (parent())
    window_origin_ += parent()->GetWindowPixelOrigin().OffsetFromOrigin();
}

Rect BaseView::GetPixelBounds() const {
  return bounds_;
}

void BaseView::SetBounds(const Rect& bounds) {
  SetPixelBounds(ScaleToEnclosingRect(bounds, scale_factor()));
}

Rect BaseView::GetBounds() {
  return ScaleToEnclosingRect(GetPixelBounds(), 1.0f / scale_factor());
}

Point BaseView::GetWindowPixelOrigin() {
  return window_origin_;
}

Rect BaseView::GetWindowPixelBounds() {
  return Rect(window_origin_, bounds_.size());
}

void BaseView::SetParent(BaseView* parent) {
  float old_scale_factor = scale_factor();

  is_content_view_ = false;
  parent_ = parent;
  window_ = parent ? parent->window_ : nullptr;

  // Scale the bounds after moving to a new parent.
  if (old_scale_factor != scale_factor()) {
    bounds_ = ScaleToEnclosingRect(bounds_, scale_factor() / old_scale_factor);
    preferred_size_ = ScaleToCeiledSize(preferred_size_,
                                        scale_factor() / old_scale_factor);
  }
}

void BaseView::BecomeContentView(WindowImpl* parent) {
  float old_scale_factor = scale_factor();

  is_content_view_ = true;
  parent_ = nullptr;
  window_ = parent;

  // Scale the bounds after moving to a new parent.
  if (old_scale_factor != scale_factor()) {
    bounds_ = ScaleToEnclosingRect(bounds_, scale_factor() / old_scale_factor);
    preferred_size_ = ScaleToCeiledSize(preferred_size_,
                                        scale_factor() / old_scale_factor);
  }
}

void BaseView::Invalidate(const Rect& dirty) {
  // Nothing to draw?
  if (!window_ || bounds_.size().IsEmpty())
    return;

  // Default to redraw whole view.
  if (dirty.IsEmpty()) {
    Invalidate(Rect(bounds_.size()));
    return;
  }

  RECT rect = (dirty + GetWindowPixelOrigin().OffsetFromOrigin()).ToRECT();
  InvalidateRect(window_->hwnd(), &rect, TRUE);
}

}  // namespace nu
