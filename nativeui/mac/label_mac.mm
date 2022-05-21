// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include "nativeui/gfx/attributed_text.h"
#include "nativeui/gfx/mac/painter_mac.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NULabel : NSView<NUViewMethods> {
 @private
  nu::NUViewPrivate private_;
  nu::Color background_color_;
}
@end

@implementation NULabel

- (void)drawRect:(NSRect)dirtyRect {
  nu::PainterMac painter(self);
  painter.SetColor(background_color_);
  painter.FillRect(nu::RectF(dirtyRect));

  auto* label = static_cast<nu::Label*>([self shell]);
  label->UpdateColor();
  painter.DrawAttributedText(label->GetAttributedText(),
                             nu::RectF(nu::SizeF([self frame].size)));
}

- (nu::NUViewPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
  [self setNeedsDisplay:YES];
}

- (void)setNUColor:(nu::Color)color {
  [self setNeedsDisplay:YES];
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

@end

namespace nu {

NativeView Label::PlatformCreate() {
  return [[NULabel alloc] init];
}

}  // namespace nu
