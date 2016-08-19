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

  if (wrapper_->child_count() > 0)
    wrapper_->child_at(0)->SetBounds(wrapper_->GetBounds());
}

- (void)resizeSubviewsWithOldSize:(NSSize)oldBoundsSize {
  [self adjustSubviews];
}

@end

namespace nu {

Container::Container() {
  set_view([[ContainerView alloc] initWithWrapper:this]);
}

Container::~Container() {
}

void Container::PlatformAddChildView(View* child) {
  View::PlatformAddChildView(child);
  [static_cast<ContainerView*>(view()) adjustSubviews];
}

void Container::PlatformRemoveChildView(View* child) {
  View::PlatformRemoveChildView(child);
  [static_cast<ContainerView*>(view()) adjustSubviews];
}

}  // namespace nu
