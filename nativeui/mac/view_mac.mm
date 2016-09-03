// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#import <Cocoa/Cocoa.h>

#include "nativeui/container.h"
#include "nativeui/gfx/mac/coordinate_conversion.h"

namespace nu {

View::View() : view_(nil) {
}

View::~View() {
  [view_ release];
}

void View::TakeOverView(NativeView view) {
  view_ = view;
}

void View::SetBounds(const Rect& bounds) {
  CGRect frame = bounds.ToCGRect();
  if (parent_) {
    frame.origin.y = parent_->GetBounds().height() -
                     (frame.origin.y + frame.size.height);
  }
  [view_ setFrame:frame];
  // Calling setFrame manually does not trigger adjustSubviews.
  [view_ resizeSubviewsWithOldSize:frame.size];
}

Rect View::GetBounds() const {
  Rect bounds(NSRectToCGRect([view_ frame]));
  if (parent_) {
    bounds.set_y(parent_->GetBounds().height() -
                 (bounds.y() + bounds.height()));
  }
  return bounds;
}

void View::SetPixelBounds(const Rect& bounds) {
  SetBounds(bounds);
}

Rect View::GetPixelBounds() const {
  return GetBounds();
}

Point View::GetWindowOrigin() const {
  if (!view_.window || !view_.window.contentView)
    return GetBounds().origin();
  NSRect contentFrame = view_.window.contentView.frame;
  Rect bounds([view_ convertRect:view_.bounds
                               toView:view_.window.contentView]);
  return Point(bounds.x(),
                    NSHeight(contentFrame) - (bounds.y() + bounds.height()));
}

Point View::GetWindowPixelOrigin() const {
  return GetWindowOrigin();
}

bool View::SetPreferredSize(const Size& size) {
  return DoSetPreferredSize(size);
}

bool View::SetPixelPreferredSize(const Size& size) {
  return DoSetPreferredSize(size);
}

Size View::GetPixelPreferredSize() const {
  return preferred_size();
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
