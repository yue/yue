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

void View::TakeOverView(NativeView view) {
  view_ = view;
}

void View::SetBounds(const Rect& bounds) {
  view_->SetBounds(bounds);
}

Rect View::GetBounds() const {
  return view_->GetBounds();
}

void View::SetPixelBounds(const Rect& bounds) {
  view_->SetPixelBounds(bounds);
}

Rect View::GetPixelBounds() const {
  return view_->GetPixelBounds();
}

Point View::GetWindowOrigin() const {
  return ScaleToFlooredPoint(GetWindowPixelOrigin(),
                             1.0f / view_->scale_factor());
}

Point View::GetWindowPixelOrigin() const {
  return view_->GetWindowPixelOrigin();
}

bool View::SetPreferredSize(const Size& size) {
  view_->set_pixel_preferred_size(
      ScaleToCeiledSize(size, view_->scale_factor()));
  return DoSetPreferredSize(size);
}

bool View::SetPixelPreferredSize(const Size& size) {
  view_->set_pixel_preferred_size(size);
  return DoSetPreferredSize(
      ScaleToCeiledSize(size, 1.0f / view_->scale_factor()));
}

Size View::GetPixelPreferredSize() const {
  return view_->pixel_preferred_size();
}

void View::Invalidate(const Rect& dirty) {
  view_->Invalidate(dirty);
}

int View::DIPToPixel(int length) const {
  return static_cast<int>(std::ceil(length * view_->scale_factor()));
}

void View::PlatformSetVisible(bool visible) {
  view_->set_visible(visible);
}

bool View::IsVisible() const {
  return view_->is_visible();
}

}  // namespace nu
