// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/scroll.h"

#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUScroll : NSScrollView<NUView> {
 @private
  nu::NUPrivate private_;
  NSSize content_size_;
}
- (void)setContentSize:(NSSize)size;
@end

@implementation NUScroll

- (nu::NUPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
}

- (void)setNUColor:(nu::Color)color {
}

- (void)setNUBackgroundColor:(nu::Color)color {
}

- (void)setNUEnabled:(BOOL)enabled {
}

- (BOOL)isNUEnabled {
  return YES;
}

- (void)setContentSize:(NSSize)size {
  content_size_ = size;
}

- (void)resizeSubviewsWithOldSize:(NSSize)oldBoundsSize {
  // Automatically resize the content view when ScrollView is larger than the
  // content size.
  NSSize parent_size = [self frame].size;
  NSSize content_size = content_size_;
  if (content_size.width < parent_size.width)
    content_size.width = parent_size.width;
  if (content_size.height < parent_size.height)
    content_size.height = parent_size.height;
  [self.documentView setFrameSize:content_size];
  [super resizeSubviewsWithOldSize:oldBoundsSize];
}

@end

namespace nu {

void Scroll::PlatformInit() {
  auto* scroll = [[NUScroll alloc] init];
  scroll.drawsBackground = NO;
  if (scroll.scrollerStyle == NSScrollerStyleOverlay) {
    scroll.hasHorizontalScroller = YES;
    scroll.hasVerticalScroller = YES;
  }
  [scroll.contentView setCopiesOnScroll:NO];
  TakeOverView(scroll);
}

void Scroll::PlatformSetContentView(View* view) {
  auto* scroll = static_cast<NSScrollView*>(GetNative());
  scroll.documentView = view->GetNative();
}

void Scroll::SetContentSize(const SizeF& size) {
  auto* scroll = static_cast<NUScroll*>(GetNative());
  NSSize content_size = size.ToCGSize();
  [scroll setContentSize:content_size];
  [scroll.documentView setFrameSize:content_size];
}

void Scroll::SetOverlayScrollbar(bool overlay) {
  auto* scroll = static_cast<NUScroll*>(GetNative());
  scroll.scrollerStyle = overlay ? NSScrollerStyleOverlay
                                 : NSScrollerStyleLegacy;
}

bool Scroll::IsOverlayScrollbar() const {
  auto* scroll = static_cast<NUScroll*>(GetNative());
  return scroll.scrollerStyle == NSScrollerStyleOverlay;
}

void Scroll::SetScrollbarPolicy(Policy h_policy, Policy v_policy) {
  auto* scroll = static_cast<NUScroll*>(GetNative());
  scroll.hasHorizontalScroller = h_policy != Policy::Never;
  scroll.hasVerticalScroller = v_policy != Policy::Never;
}

std::tuple<Scroll::Policy, Scroll::Policy> Scroll::GetScrollbarPolicy() const {
  auto* scroll = static_cast<NUScroll*>(GetNative());
  Policy h_policy = scroll.hasHorizontalScroller ? Policy::Automatic
                                                 : Policy::Never;
  Policy v_policy = scroll.hasVerticalScroller ? Policy::Automatic
                                               : Policy::Never;
  return std::make_tuple(h_policy, v_policy);
}

}  // namespace nu
