// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#import <Cocoa/Cocoa.h>

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

gfx::Rect View::GetBounds() {
  gfx::Rect bounds(NSRectToCGRect([view_ frame]));
  if (parent_) {
    bounds.set_y(parent_->GetBounds().height() -
                 (bounds.y() + bounds.height()));
  }
  return bounds;
}

}  // namespace nu
