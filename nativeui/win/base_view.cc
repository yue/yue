// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/base_view.h"

#include "nativeui/win/screen.h"

namespace nu {

BaseWindow::BaseWindow(HWND hwnd)
    : hwnd_(hwnd), scale_factor_(GetScaleFactorForHWND(hwnd)) {
}

void BaseView::SetPixelBounds(const gfx::Rect& bounds) {
  bounds_ = bounds;
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

void BaseView::SetParent(BaseView* parent) {
  float old_scale_factor = scale_factor();

  is_content_view_ = false;
  parent_ = parent;
  window_ = parent ? parent->window_ : nullptr;

  // Scale the bounds after moving to a new parent.
  if (old_scale_factor != scale_factor())
    bounds_ = ScaleToEnclosingRect(bounds_, scale_factor() / old_scale_factor);
}

void BaseView::BecomeContentView(BaseWindow* parent) {
  float old_scale_factor = scale_factor();

  is_content_view_ = true;
  parent_ = nullptr;
  window_ = parent;

  // Scale the bounds after moving to a new parent.
  if (old_scale_factor != scale_factor())
    bounds_ = ScaleToEnclosingRect(bounds_, scale_factor() / old_scale_factor);
}

}  // namespace nu
