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
  SetPixelBounds(ScaleToEnclosingRect(bounds, view()->scale_factor()));
}

Rect View::GetBounds() const {
  return ScaleToEnclosingRect(GetPixelBounds(), 1.0f / view()->scale_factor());
}

void View::SetPixelBounds(const Rect& bounds) {
  Rect size_allocation(bounds);
  if (parent())
    size_allocation += parent()->view()->size_allocation().OffsetFromOrigin();
  view()->SizeAllocate(size_allocation);
}

Rect View::GetPixelBounds() const {
  Rect bounds(view()->size_allocation());
  if (parent())
    bounds -= parent()->view()->size_allocation().OffsetFromOrigin();
  return bounds;
}

Point View::GetWindowOrigin() const {
  return ScaleToFlooredPoint(GetWindowPixelOrigin(),
                             1.0f / view()->scale_factor());
}

Point View::GetWindowPixelOrigin() const {
  return view()->size_allocation().origin();
}

bool View::SetPreferredSize(const Size& size) {
  view()->set_preferred_size(ScaleToCeiledSize(size, view()->scale_factor()));
  return DoSetPreferredSize(size);
}

bool View::SetPixelPreferredSize(const Size& size) {
  view()->set_preferred_size(size);
  return DoSetPreferredSize(ScaleToCeiledSize(size,
                                              1.0f / view()->scale_factor()));
}

Size View::GetPixelPreferredSize() const {
  return view()->preferred_size();
}

int View::DIPToPixel(int length) const {
  return static_cast<int>(std::ceil(length * view()->scale_factor()));
}

void View::PlatformSetVisible(bool visible) {
  view()->set_visible(visible);
}

bool View::IsVisible() const {
  return view()->is_visible();
}

}  // namespace nu
