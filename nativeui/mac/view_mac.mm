// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#import <Cocoa/Cocoa.h>

#include "nativeui/container.h"
#include "nativeui/gfx/geometry/point_conversions.h"
#include "nativeui/gfx/geometry/rect_conversions.h"
#include "nativeui/gfx/mac/coordinate_conversion.h"

namespace nu {

void View::PlatformDestroy() {
  [view_ release];
}

void View::TakeOverView(NativeView view) {
  view_ = view;
}

void View::SetBounds(const RectF& bounds) {
  CGRect frame = bounds.ToCGRect();
  if (parent_) {
    frame.origin.y = parent_->GetBounds().height() -
                     (frame.origin.y + frame.size.height);
  }
  [view_ setFrame:frame];
  // Calling setFrame manually does not trigger adjustSubviews.
  [view_ resizeSubviewsWithOldSize:frame.size];
}

RectF View::GetBounds() const {
  RectF bounds(NSRectToCGRect([view_ frame]));
  if (parent_) {
    bounds.set_y(parent_->GetBounds().height() -
                 (bounds.y() + bounds.height()));
  }
  return bounds;
}

void View::SetPixelBounds(const Rect& bounds) {
  SetBounds(RectF(bounds));
}

Rect View::GetPixelBounds() const {
  return ToNearestRect(GetBounds());
}

PointF View::GetWindowOrigin() const {
  if (!view_.window || !view_.window.contentView)
    return GetBounds().origin();
  NSRect contentFrame = view_.window.contentView.frame;
  RectF bounds([view_ convertRect:view_.bounds
                           toView:view_.window.contentView]);
  return PointF(bounds.x(),
                NSHeight(contentFrame) - (bounds.y() + bounds.height()));
}

Point View::GetWindowPixelOrigin() const {
  return ToRoundedPoint(GetWindowOrigin());
}

int View::DIPToPixel(int length) const {
  return length;
}

void View::PlatformSetVisible(bool visible) {
  [view_ setHidden:!visible];
}

bool View::IsVisible() const {
  return ![view_ isHidden];
}

}  // namespace nu
