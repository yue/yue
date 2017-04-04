// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/mac/painter_mac.h"
#include "nativeui/gfx/text.h"
#include "nativeui/mac/view_mac.h"
#include "nativeui/state.h"

@interface NULabel : NSView<NUView> {
 @private
  nu::Label* shell_;
  std::string text_;
  nu::Color background_color_;
}
- (id)initWithShell:(nu::Label*)shell;
- (nu::View*)shell;
- (void)setNUBackgroundColor:(nu::Color)color;
- (void)setText:(const std::string&)text;
- (std::string)text;
@end

@implementation NULabel

- (id)initWithShell:(nu::Label*)shell {
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

- (void)setText:(const std::string&)text {
  text_ = text;
  [self setNeedsDisplay:YES];
}

- (std::string)text {
  return text_;
}

- (void)drawRect:(NSRect)dirtyRect {
  nu::PainterMac painter;
  painter.SetColor(background_color_);
  painter.FillRect(nu::RectF(dirtyRect));
  painter.DrawTextWithFlags(text_, nu::State::GetCurrent()->GetDefaultFont(),
                              nu::RectF([self frame]),
                              nu::Painter::TextAlignCenter);
}

@end

namespace nu {

namespace {

SizeF GetPreferredSizeForText(const std::string& text) {
  SizeF size = MeasureText(text, nu::State::GetCurrent()->GetDefaultFont());
  size.Enlarge(1, 1);  // leave space for border
  return size;
}

}  // namespace

Label::Label(const std::string& text) {
  TakeOverView([[NULabel alloc] init]);
  SetText(text);
}

Label::~Label() {
}

void Label::SetText(const std::string& text) {
  [static_cast<NULabel*>(GetNative()) setText:text];
  SetDefaultStyle(GetPreferredSizeForText(text));
}

std::string Label::GetText() {
  return [static_cast<NULabel*>(GetNative()) text];
}

}  // namespace nu
