// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#include "nativeui/gfx/geometry/rect_conversions.h"
#include "nativeui/label.h"
#include "nativeui/win/base_view.h"

namespace nu {

void View::PlatformDestroy() {
  delete view_;
}

void View::TakeOverView(NativeView view) {
  view_ = view;
}

void View::SetBounds(const RectF& bounds) {
  SetPixelBounds(ToEnclosingRect(ScaleRect(bounds, view()->scale_factor())));
}

RectF View::GetBounds() const {
  return ScaleRect(RectF(GetPixelBounds()), 1.0f / view()->scale_factor());
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

void View::PlatformSetVisible(bool visible) {
  view()->set_visible(visible);
}

bool View::IsVisible() const {
  return view()->is_visible();
}

void View::SetBackgroundColor(Color color) {
  view()->SetBackgroundColor(color);
}

}  // namespace nu
