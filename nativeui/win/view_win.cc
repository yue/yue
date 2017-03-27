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
  SetPixelBounds(ToEnclosingRect(ScaleRect(bounds,
                                 GetNative()->scale_factor())));
}

RectF View::GetBounds() const {
  return ScaleRect(RectF(GetPixelBounds()), 1.0f / GetNative()->scale_factor());
}

void View::SetPixelBounds(const Rect& bounds) {
  Rect size_allocation(bounds);
  if (GetParent()) {
    size_allocation +=
        GetParent()->GetNative()->size_allocation().OffsetFromOrigin();
  }
  GetNative()->SizeAllocate(size_allocation);
}

Rect View::GetPixelBounds() const {
  Rect bounds(GetNative()->size_allocation());
  if (GetParent())
    bounds -= GetParent()->GetNative()->size_allocation().OffsetFromOrigin();
  return bounds;
}

void View::PlatformSetVisible(bool visible) {
  GetNative()->set_visible(visible);
}

bool View::IsVisible() const {
  return GetNative()->is_visible();
}

void View::PlatformSetBackgroundColor(Color color) {
  GetNative()->SetBackgroundColor(color);
}

}  // namespace nu
