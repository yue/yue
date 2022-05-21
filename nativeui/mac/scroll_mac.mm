// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/scroll.h"

#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUScroll : NSScrollView<NUViewMethods> {
 @private
  nu::NUViewPrivate private_;
  nu::Scroll* shell_;
  BOOL subscribed_;
  NSSize content_size_;
}
- (id)initWithShell:(nu::Scroll*)shell;
- (void)setEventEnabled:(BOOL)enable;
- (void)setContentSize:(NSSize)size;
@end

@implementation NUScroll

- (id)initWithShell:(nu::Scroll*)shell {
  if ((self = [super init])) {
    shell_ = shell;
    subscribed_ = NO;
  }
  return self;
}

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
  [super dealloc];
}

- (void)setEventEnabled:(BOOL)enable {
  if (subscribed_ == enable)
    return;
  subscribed_ = enable;
  NSView* contentView = self.contentView;
  if (enable) {
    [contentView setPostsBoundsChangedNotifications:YES];
    [[NSNotificationCenter defaultCenter]
        addObserver:self
           selector:@selector(onScroll:)
               name:NSViewBoundsDidChangeNotification
             object:contentView];
  } else {
    [contentView setPostsBoundsChangedNotifications:NO];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
  }
}

- (void)setContentSize:(NSSize)size {
  content_size_ = size;
}

- (void)onScroll:(NSNotification*)notification {
  shell_->on_scroll.Emit(shell_);
}

- (nu::NUViewPrivate*)nuPrivate {
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
  auto* scroll = [[NUScroll alloc] initWithShell:this];
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

void Scroll::SetScrollPosition(float horizon, float vertical) {
  auto* scroll = static_cast<NUScroll*>(GetNative());
  if (![scroll.documentView isFlipped])
    vertical = NSHeight(scroll.documentView.bounds) - vertical;
  [scroll.documentView scrollPoint:NSMakePoint(horizon, vertical)];
}

std::tuple<float, float> Scroll::GetScrollPosition() const {
  auto* scroll = static_cast<NUScroll*>(GetNative());
  NSPoint point = scroll.contentView.bounds.origin;
  if (![scroll.documentView isFlipped]) {
    point.y = NSHeight(scroll.documentView.bounds) -
              NSHeight(scroll.contentView.bounds) -
              point.y;
  }
  return std::make_tuple(point.x, point.y);
}

std::tuple<float, float> Scroll::GetMaximumScrollPosition() const {
  auto* scroll = static_cast<NUScroll*>(GetNative());
  NSRect docBounds = scroll.documentView.bounds;
  NSRect clipBounds = scroll.contentView.bounds;
  return std::make_tuple(NSMaxX(docBounds) - NSWidth(clipBounds),
                         NSMaxY(docBounds) - NSHeight(clipBounds));
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

void Scroll::SetScrollElasticity(Elasticity h, Elasticity v) {
  auto* scroll = static_cast<NUScroll*>(GetNative());
  scroll.horizontalScrollElasticity = static_cast<NSScrollElasticity>(h);
  scroll.verticalScrollElasticity = static_cast<NSScrollElasticity>(v);
}

std::tuple<Scroll::Elasticity, Scroll::Elasticity> Scroll::GetScrollElasticity() const {
  auto* scroll = static_cast<NUScroll*>(GetNative());
  Elasticity h = static_cast<Elasticity>(scroll.horizontalScrollElasticity);
  Elasticity v = static_cast<Elasticity>(scroll.verticalScrollElasticity);
  return std::make_tuple(h, v);
}

void Scroll::SubscribeOnScroll() {
  auto* scroll = static_cast<NUScroll*>(GetNative());
  [scroll setEventEnabled:YES];
}

}  // namespace nu
