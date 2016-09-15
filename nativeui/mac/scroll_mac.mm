// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/scroll.h"

#import <Cocoa/Cocoa.h>

// The NSScrollView tiles the document view from bottom-left by default, by
// providing a flipped content view, we can make NSScrollView tiles the document
// view from top-left.
@interface NUFlippedView : NSClipView
@end

@implementation NUFlippedView

- (BOOL)isFlipped {
  return YES;
}

- (BOOL)drawsBackground {
  return NO;
}

@end

namespace nu {

void Scroll::PlatformInit() {
  auto* scroll = [[NSScrollView alloc] init];
  scroll.drawsBackground = NO;
  scroll.contentView = [[[NUFlippedView alloc] init] autorelease];
  TakeOverView(scroll);
}

void Scroll::PlatformSetContentView(Container* container) {
  auto* scroll = static_cast<NSScrollView*>(view());
  scroll.documentView = container->view();
}

void Scroll::SetContentSize(const SizeF& size) {
  GetContentView()->SetBounds(RectF(size));
}

void Scroll::SetScrollBarPolicy(Policy h_policy, Policy v_policy) {
  auto* scroll = static_cast<NSScrollView*>(view());
  scroll.hasHorizontalScroller = (h_policy == Policy::Always) ||
                                 (h_policy == Policy::Automatic);
  scroll.hasVerticalScroller = (v_policy == Policy::Always) ||
                               (v_policy == Policy::Automatic);
}

std::tuple<Scroll::Policy, Scroll::Policy> Scroll::GetScrollBarPolicy() const {
  auto* scroll = static_cast<NSScrollView*>(view());
  Policy h_policy = scroll.hasHorizontalScroller ? Policy::Automatic
                                                 : Policy::Never;
  Policy v_policy = scroll.hasVerticalScroller ? Policy::Automatic
                                               : Policy::Never;
  return std::make_tuple(h_policy, v_policy);
}

}  // namespace nu
