// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/font.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/mac/painter_mac.h"
#include "nativeui/gfx/mac/text_mac.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"
#include "nativeui/system.h"

@interface NULabel : NSView<NUView> {
 @private
  nu::NUPrivate private_;
  std::string text_;
  nu::TextAlign align_;
  nu::TextAlign valign_;
  scoped_refptr<nu::Font> font_;
  nu::Color color_;
  nu::Color background_color_;
}
- (void)setText:(const std::string&)text;
- (std::string)text;
- (void)setTextAlign:(nu::TextAlign)align;
- (void)setTextVAlign:(nu::TextAlign)align;
@end

@implementation NULabel

- (id)init {
  if (self = [super init]) {
    align_ = nu::TextAlign::Center;
    valign_ = nu::TextAlign::Center;
  }
  return self;
}

- (void)setText:(const std::string&)text {
  text_ = text;
  [self setNeedsDisplay:YES];
}

- (std::string)text {
  return text_;
}

- (void)setTextAlign:(nu::TextAlign)align {
  align_ = align;
  [self setNeedsDisplay:YES];
}

- (void)setTextVAlign:(nu::TextAlign)align {
  valign_ = align;
  [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect {
  nu::PainterMac painter;
  painter.SetColor(background_color_);
  painter.FillRect(nu::RectF(dirtyRect));
  nu::TextAttributes attributes(font_.get(), color_, align_, valign_);
  painter.DrawText(text_, nu::RectF(nu::SizeF([self frame].size)), attributes);
}

- (nu::NUPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
  font_ = font;
  [self setNeedsDisplay:YES];
}

- (void)setNUColor:(nu::Color)color {
  color_ = color;
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

Label::Label(const std::string& text) {
  TakeOverView([[NULabel alloc] init]);
  SetText(text);
  // Default styles.
  [GetNative() setNUFont:System::GetDefaultFont()];
  [GetNative() setNUColor:System::GetColor(System::Color::Text)];
}

Label::~Label() {
}

void Label::PlatformSetText(const std::string& text) {
  [static_cast<NULabel*>(GetNative()) setText:text];
}

std::string Label::GetText() const {
  return [static_cast<NULabel*>(GetNative()) text];
}

void Label::SetAlign(TextAlign align) {
  [static_cast<NULabel*>(GetNative()) setTextAlign:align];
}

void Label::SetVAlign(TextAlign align) {
  [static_cast<NULabel*>(GetNative()) setTextVAlign:align];
}

SizeF Label::GetMinimumSize() const {
  TextMetrics metrics = MeasureText(GetText(), -1, TextAttributes());
  metrics.size.Enlarge(1, 1);  // leave space for border
  return metrics.size;
}

}  // namespace nu
