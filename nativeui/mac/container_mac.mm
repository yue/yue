// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#import <Cocoa/Cocoa.h>

#include "base/logging.h"

@interface ContainerView : NSView {
 @private
  nu::Container* wrapper_;
  nu::Color background_color_;
}
- (id)initWithWrapper:(nu::Container*)wrapper;
- (void)setBackgroundColor:(nu::Color)color;
@end

@implementation ContainerView

- (id)initWithWrapper:(nu::Container*)wrapper {
  self = [super init];
  if (!self)
    return nil;

  wrapper_ = wrapper;
  return self;
}

- (void)setBackgroundColor:(nu::Color)color {
  background_color_ = color;
  [self setNeedsDisplay:YES];
}

- (void)adjustSubviews {
  DCHECK_EQ(static_cast<int>([[self subviews] count]), wrapper_->child_count())
      << "Subviews do not match children views";

  wrapper_->BoundsChanged();
}

- (BOOL)isFlipped {
  return YES;
}

- (void)resizeSubviewsWithOldSize:(NSSize)oldBoundsSize {
  [self adjustSubviews];
}

- (void)drawRect:(NSRect)rect {
  if (background_color_.a() != 0) {
    [background_color_.ToNSColor() setFill];
    NSRectFill(rect);
  }
  [super drawRect:rect];
}

@end

namespace nu {

void Container::PlatformInit() {
  TakeOverView([[ContainerView alloc] initWithWrapper:this]);
}

void Container::PlatformDestroy() {
}

void Container::SetBackgroundColor(Color color) {
  auto* container = static_cast<ContainerView*>(view());
  [container setBackgroundColor:color];
}

void Container::PlatformAddChildView(View* child) {
  [view() addSubview:child->view()];
}

void Container::PlatformRemoveChildView(View* child) {
  [child->view() removeFromSuperview];
}

}  // namespace nu
