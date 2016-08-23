// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#include "nativeui/win/base_view.h"

namespace nu {

View::View() : view_(nullptr) {
}

View::~View() {
  delete view_;
}

void View::SetBounds(const gfx::Rect& bounds) {
  view_->SetBounds(bounds);
}

gfx::Rect View::GetBounds() const {
  return view_->GetBounds();
}

void View::SetPixelBounds(const gfx::Rect& bounds) {
  view_->SetPixelBounds(bounds);
}

gfx::Rect View::GetPixelBounds() const {
  return view_->GetPixelBounds();
}

gfx::Point View::GetWindowOrigin() const {
  return ScaleToFlooredPoint(GetWindowPixelOrigin(),
                             1.0f / view_->scale_factor());
}

gfx::Point View::GetWindowPixelOrigin() const {
  return view_->GetWindowPixelOrigin();
}

}  // namespace nu
