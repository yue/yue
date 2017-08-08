// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/text_edit.h"

#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/font.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUTextEdit : NSScrollView<NUView> {
 @private
  base::scoped_nsobject<NSTextView> textView_;
  nu::NUPrivate private_;
}
- (nu::NUPrivate*)nuPrivate;
- (void)setNUFont:(nu::Font*)font;
- (void)setNUColor:(nu::Color)color;
- (void)setNUBackgroundColor:(nu::Color)color;
@end

@implementation NUTextEdit

- (id)init {
  if ((self = [super init])) {
    textView_.reset([[NSTextView alloc] init]);
    [textView_ setRichText:NO];
    [textView_ setHorizontallyResizable:YES];
    [textView_ setVerticallyResizable:YES];
    [textView_ setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
    [[textView_ textContainer] setContainerSize:NSMakeSize(FLT_MAX, FLT_MAX)];
    [[textView_ textContainer] setWidthTracksTextView:YES];
    self.documentView = textView_.get();
  }
  return self;
}

- (nu::NUPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
  [textView_ setFont:font->GetNative()];
}

- (void)setNUColor:(nu::Color)color {
  [textView_ setTextColor:color.ToNSColor()];
}

- (void)setNUBackgroundColor:(nu::Color)color {
  [textView_ setBackgroundColor:color.ToNSColor()];
}

@end

namespace nu {

TextEdit::TextEdit() {
  NUTextEdit* edit = [[NUTextEdit alloc] init];
  [edit setBorderType:NSNoBorder];
  [edit setHasVerticalScroller:YES];
  [edit setHasHorizontalScroller:YES];
  TakeOverView(edit);
}

TextEdit::~TextEdit() {
}

std::string TextEdit::GetText() const {
  auto* edit = static_cast<NUTextEdit*>(GetNative());
  auto* text_view = static_cast<NSTextView*>([edit documentView]);
  return base::SysNSStringToUTF8([[text_view textStorage] string]);
}

}  // namespace nu
