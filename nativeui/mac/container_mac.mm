// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#import <Cocoa/Cocoa.h>

#include "base/logging.h"

@interface ContainerView : NSView {
 @private
  nu::Container* wrapper_;
}
- (id)initWithWrapper:(nu::Container*)wrapper;
@end

@implementation ContainerView

- (id)initWithWrapper:(nu::Container*)wrapper {
  self = [super init];
  if (!self)
    return nil;

  self->wrapper_ = wrapper;
  return self;
}

- (void)adjustSubviews {
  DCHECK_EQ(static_cast<int>([[self subviews] count]), wrapper_->child_count())
      << "Subviews do not match children views";

  wrapper_->BoundsChanged();
}

- (void)resizeSubviewsWithOldSize:(NSSize)oldBoundsSize {
  [self adjustSubviews];
}

@end

namespace nu {

void Container::PlatformInit() {
  TakeOverView([[ContainerView alloc] initWithWrapper:this]);
}

void Container::PlatformDestroy() {
}

void Container::PlatformAddChildView(View* child) {
  [view() addSubview:child->view()];
}

void Container::PlatformRemoveChildView(View* child) {
  [child->view() removeFromSuperview];
}

}  // namespace nu
