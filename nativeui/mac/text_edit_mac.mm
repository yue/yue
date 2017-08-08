// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/text_edit.h"

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/font.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUTextEdit : NSTextView<NUView> {
 @private
  nu::NUPrivate private_;
}
- (nu::NUPrivate*)nuPrivate;
- (void)setNUFont:(nu::Font*)font;
- (void)setNUColor:(nu::Color)color;
- (void)setNUBackgroundColor:(nu::Color)color;
@end

@implementation NUTextEdit

- (nu::NUPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
  [self setFont:font->GetNative()];
}

- (void)setNUColor:(nu::Color)color {
  [self setTextColor:color.ToNSColor()];
}

- (void)setNUBackgroundColor:(nu::Color)color {
  [self setBackgroundColor:color.ToNSColor()];
}

@end

namespace nu {

TextEdit::TextEdit() {
  NSTextView* text_view = [[NUTextEdit alloc] init];
  TakeOverView(text_view);
}

TextEdit::~TextEdit() {
}

std::string TextEdit::GetText() const {
  auto* text_view = static_cast<NSTextView*>(GetNative());
  return base::SysNSStringToUTF8([[text_view textStorage] string]);
}

}  // namespace nu
