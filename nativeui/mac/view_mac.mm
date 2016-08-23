// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#import <Cocoa/Cocoa.h>

#include "nativeui/container.h"
#include "ui/gfx/mac/coordinate_conversion.h"

namespace nu {

View::View() : view_(nil) {
}

View::~View() {
  [view_ release];
}

void View::SetBounds(const gfx::Rect& bounds) {
  CGRect frame = bounds.ToCGRect();
  if (parent_) {
    frame.origin.y = parent_->GetBounds().height() -
                     (frame.origin.y + frame.size.height);
  }
  [view_ setFrame:frame];
}

gfx::Rect View::GetBounds() const {
  gfx::Rect bounds(NSRectToCGRect([view_ frame]));
  if (parent_) {
    bounds.set_y(parent_->GetBounds().height() -
                 (bounds.y() + bounds.height()));
  }
  return bounds;
}

void View::SetPixelBounds(const gfx::Rect& bounds) {
  SetBounds(bounds);
}

gfx::Rect View::GetPixelBounds() const {
  return GetBounds();
}

gfx::Point View::GetWindowOrigin() const {
  if (!view_.window || !view_.window.contentView)
    return GetBounds().origin();
  NSRect contentFrame = view_.window.contentView.frame;
  gfx::Rect bounds([view_ convertRect:view_.bounds
                               toView:view_.window.contentView]);
  return gfx::Point(bounds.x(),
                    NSHeight(contentFrame) - (bounds.y() + bounds.height()));
}

gfx::Point View::GetWindowPixelOrigin() const {
  return GetWindowOrigin();
}

}  // namespace nu
