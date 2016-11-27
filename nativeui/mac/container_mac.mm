// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#include "base/logging.h"
#include "nativeui/gfx/mac/painter_mac.h"
#include "nativeui/mac/view_mac.h"

@interface NUContainer : NSView<BaseView> {
 @private
  nu::Container* shell_;
  nu::Color background_color_;
}
- (id)initWithShell:(nu::Container*)shell;
- (nu::View*)shell;
- (void)setNUBackgroundColor:(nu::Color)color;
@end

@implementation NUContainer

- (id)initWithShell:(nu::Container*)shell {
  self = [super init];
  if (!self)
    return nil;

  shell_ = shell;
  return self;
}

- (nu::View*)shell {
  return shell_;
}

- (void)setNUBackgroundColor:(nu::Color)color {
  background_color_ = color;
  [self setNeedsDisplay:YES];
}

- (void)adjustSubviews {
  DCHECK_EQ(static_cast<int>([[self subviews] count]), shell_->child_count())
      << "Subviews do not match children views";

  shell_->BoundsChanged();
}

- (BOOL)isFlipped {
  return YES;
}

- (void)resizeSubviewsWithOldSize:(NSSize)oldBoundsSize {
  [self adjustSubviews];
}

- (void)drawRect:(NSRect)dirtyRect {
  nu::RectF dirty(dirtyRect);
  nu::PainterMac painter;
  painter.SetColor(background_color_);
  painter.FillRect(dirty);
  shell_->on_draw.Emit(shell_, &painter, dirty);
}

@end

namespace nu {

void Container::PlatformInit() {
  TakeOverView([[NUContainer alloc] initWithShell:this]);
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
