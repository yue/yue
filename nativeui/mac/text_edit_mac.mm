// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/text_edit.h"

#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/font.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUTextViewDelegate : NSObject<NSTextViewDelegate> {
 @private
  nu::TextEdit* shell_;
}
- (id)initWithShell:(nu::TextEdit*)shell;
@end

@implementation NUTextViewDelegate

- (id)initWithShell:(nu::TextEdit*)shell {
  if ((self = [super init]))
    shell_ = shell;
  return self;
}

- (void)textDidChange:(NSNotification*)notification {
  shell_->on_text_change.Emit(shell_);
}

@end

@interface NUTextEdit : NSScrollView<NUView> {
 @private
  base::scoped_nsobject<NSTextView> textView_;
  base::scoped_nsobject<NUTextViewDelegate> delegate_;
  nu::NUPrivate private_;
}
- (id)initWithShell:(nu::TextEdit*)shell;
- (nu::NUPrivate*)nuPrivate;
- (void)setNUFont:(nu::Font*)font;
- (void)setNUColor:(nu::Color)color;
- (void)setNUBackgroundColor:(nu::Color)color;
@end

@implementation NUTextEdit

- (id)initWithShell:(nu::TextEdit*)shell {
  if ((self = [super init])) {
    delegate_.reset([[NUTextViewDelegate alloc] initWithShell:shell]);
    textView_.reset([[NSTextView alloc] init]);
    [textView_ setDelegate:delegate_.get()];
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
  NUTextEdit* edit = [[NUTextEdit alloc] initWithShell:this];
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
