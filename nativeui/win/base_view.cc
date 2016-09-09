// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/base_view.h"

namespace nu {

void BaseView::SetPixelBounds(const Rect& bounds) {
  if (bounds_ == bounds)
    return;

  // Old invalidate bounds.
  Rect old_bounds = Rect(bounds_.size()) +
                    GetWindowPixelOrigin().OffsetFromOrigin();

  bounds_ = bounds;

  // Update the origin to parent HWND.
  window_origin_ = bounds.origin();
  if (parent())
    window_origin_ += parent()->GetWindowPixelOrigin().OffsetFromOrigin();

  // Refresh the old bounds.
  if (window_ && !old_bounds.size().IsEmpty()) {
    RECT rect = old_bounds.ToRECT();
    InvalidateRect(window_->hwnd(), &rect, TRUE);
  }

  // Refresh the new bounds.
  Invalidate(bounds);
}

Rect BaseView::GetPixelBounds() const {
  return bounds_;
}

Size BaseView::GetPixelSize() const {
  return GetPixelBounds().size();
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

void BaseView::SetFocus(bool focus) {
  is_focused_ = focus;
  Invalidate();
}

bool BaseView::IsFocused() const {
  return is_focused_;
}

}  // namespace nu
