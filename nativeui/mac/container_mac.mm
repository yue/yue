// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/container_mac.h"

#include "nativeui/gfx/mac/painter_mac.h"
#include "nativeui/mac/nu_responder.h"

@implementation NUContainer

- (nu::NUViewPrivate*)nuPrivate {
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

- (void)setNUEnabled:(BOOL)enabled {
}

- (BOOL)isNUEnabled {
  return YES;
}

- (BOOL)isFlipped {
  return YES;
}

- (void)drawRect:(NSRect)dirtyRect {
  nu::Container* shell = static_cast<nu::Container*>([self shell]);
  if (!shell)
    return;

  nu::RectF dirty(dirtyRect);
  nu::PainterMac painter(self);
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
  // Handle the wants_layer_infected routine, which makes every view in
  // relationship with this view has wantsLayer set to true.
  NUViewPrivate* priv = [GetNative() nuPrivate];
  if (priv->wants_layer_infected) {
    [child->GetNative() setWantsLayer:YES];
  } else {
    if (IsNUResponder(child->GetNative()) &&
        [child->GetNative() nuPrivate]->wants_layer_infected) {
      // Just got infected, set wantsLayer on all children.
      priv->wants_layer_infected = true;
      SetWantsLayer(true);
      for (int i = 0; i < ChildCount(); ++i)
        [ChildAt(i)->GetNative() setWantsLayer:YES];
    }
  }
}

void Container::PlatformRemoveChildView(View* child) {
  [child->GetNative() removeFromSuperview];
  // Revert wantsLayer to default.
  NSView* nc = child->GetNative();
  if (IsNUResponder(nc))
    [nc setWantsLayer:[nc nuPrivate]->wants_layer];
  else
    [nc setWantsLayer:NO];
}

}  // namespace nu
