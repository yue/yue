// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/container_mac.h"

#include "base/logging.h"
#include "nativeui/gfx/mac/painter_mac.h"

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
  [self setNeedsDisplay:YES];
}

- (void)adjustSubviews {
  nu::Container* shell = static_cast<nu::Container*>([self shell]);
  DCHECK_EQ(static_cast<int>([[self subviews] count]), shell->ChildCount())
      << "Subviews do not match children views";

  shell->BoundsChanged();
}

- (BOOL)isFlipped {
  return YES;
}

- (void)resizeSubviewsWithOldSize:(NSSize)oldBoundsSize {
  [self adjustSubviews];
}

- (void)drawRect:(NSRect)dirtyRect {
  nu::Container* shell = static_cast<nu::Container*>([self shell]);
  nu::RectF dirty(dirtyRect);
  nu::PainterMac painter;
  painter.SetColor(shell->GetBackgroundColor());
  painter.FillRect(dirty);
  shell->on_draw.Emit(shell_, &painter, dirty);
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
