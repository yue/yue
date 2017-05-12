// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/container_mac.h"

#include "base/logging.h"
#include "nativeui/gfx/mac/painter_mac.h"

@implementation NUContainer

- (nu::NUPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
}

- (void)setNUColor:(nu::Color)color {
}

- (void)setNUBackgroundColor:(nu::Color)color {
  background_color_ = color;
  [self setNeedsDisplay:YES];
}

- (BOOL)isFlipped {
  return YES;
}

- (void)drawRect:(NSRect)dirtyRect {
  nu::Container* shell = static_cast<nu::Container*>([self shell]);
  nu::RectF dirty(dirtyRect);
  nu::PainterMac painter;
  painter.SetColor(background_color_);
  painter.FillRect(dirty);
  shell->on_draw.Emit(shell, &painter, dirty);
}

@end

namespace nu {

void Container::PlatformInit() {
  TakeOverView([[NUContainer alloc] init]);
}

void Container::PlatformDestroy() {
}

void Container::PlatformAddChildView(View* child) {
  [GetNative() addSubview:child->GetNative()];
}

void Container::PlatformRemoveChildView(View* child) {
  [child->GetNative() removeFromSuperview];
}

}  // namespace nu
