// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/text_edit.h"

#include "base/apple/scoped_nsobject.h"
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

- (BOOL)textView:(NSTextView*)textView
    doCommandBySelector:(SEL)commandSelector {
  if (commandSelector == @selector(insertNewline:) &&
      shell_->should_insert_new_line)
    return !shell_->should_insert_new_line(shell_);
  return NO;
}


@end

@interface NUTextEdit : NSScrollView<NUViewMethods> {
 @private
  base::apple::scoped_nsobject<NSTextView> textView_;
  base::apple::scoped_nsobject<NUTextViewDelegate> delegate_;
  nu::NUViewPrivate private_;
}
- (id)initWithShell:(nu::TextEdit*)shell;
@end

@implementation NUTextEdit

- (id)initWithShell:(nu::TextEdit*)shell {
  if ((self = [super init])) {
    self.drawsBackground = NO; // disable super NSScrollView background
    delegate_.reset([[NUTextViewDelegate alloc] initWithShell:shell]);
    textView_.reset([[NSTextView alloc] init]);
    [textView_.get() setDelegate:delegate_.get()];
    [textView_.get() setRichText:NO];
    [textView_.get() setAllowsUndo:YES];
    // Do not change width to fix text, i.e. alwasys wrap text.
    [textView_.get() setHorizontallyResizable:NO];
    [textView_.get() setVerticallyResizable:YES];
    [textView_.get() setTextContainerInset:NSMakeSize(0, 0)];
    [textView_.get() setAutoresizingMask:NSViewWidthSizable];
    [[textView_.get() textContainer] setContainerSize:NSMakeSize(FLT_MAX, FLT_MAX)];
    [[textView_.get() textContainer] setWidthTracksTextView:YES];
    [[textView_.get() textContainer] setLineFragmentPadding:0];
    self.documentView = textView_.get();
  }
  return self;
}

- (nu::NUViewPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
  [textView_.get() setFont:font->GetNative()];
}

- (void)setNUColor:(nu::Color)color {
  [textView_.get() setTextColor:color.ToNSColor()];
}

- (void)setNUBackgroundColor:(nu::Color)color {
  [textView_.get() setBackgroundColor:color.ToNSColor()];
}

- (void)setNUEnabled:(BOOL)enabled {
  [textView_.get() setEditable:enabled];
}

- (BOOL)isNUEnabled {
  return [textView_.get() isEditable];
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

void TextEdit::SetText(const std::string& text) {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  [textView setString:base::SysUTF8ToNSString(text)];
}

std::string TextEdit::GetText() const {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  return base::SysNSStringToUTF8([[textView textStorage] string]);
}

void TextEdit::Redo() {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  [[textView undoManager] redo];
}

bool TextEdit::CanRedo() const {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  return [[textView undoManager] canRedo];
}

void TextEdit::Undo() {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  [[textView undoManager] undo];
}

bool TextEdit::CanUndo() const {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  return [[textView undoManager] canUndo];
}

void TextEdit::Cut() {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  [textView cut:nil];
}

void TextEdit::Copy() {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  [textView copy:nil];
}

void TextEdit::Paste() {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  [textView paste:nil];
}

void TextEdit::SelectAll() {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  [textView selectAll:nil];
}

std::tuple<int, int> TextEdit::GetSelectionRange() const {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  NSRange range = [textView selectedRange];
  return std::make_tuple<int, int>(
      range.location, range.location + range.length);
}

void TextEdit::SelectRange(int start, int end) {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  [textView setSelectedRange:NSMakeRange(start, end - start)];
}

std::string TextEdit::GetTextInRange(int start, int end) const {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  NSString* str = [[textView textStorage] string];
  return base::SysNSStringToUTF8(
      [str substringWithRange:NSMakeRange(start, end - start)]);
}

void TextEdit::InsertText(const std::string& text) {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  [textView insertText:base::SysUTF8ToNSString(text)
       replacementRange:[textView selectedRange]];
}

void TextEdit::InsertTextAt(const std::string& text, int pos) {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  [textView insertText:base::SysUTF8ToNSString(text)
       replacementRange:NSMakeRange(pos, 0)];
}

void TextEdit::Delete() {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  [textView delete:nil];
}

void TextEdit::DeleteRange(int start, int end) {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  [textView insertText:@""
       replacementRange:NSMakeRange(start, end - start)];
}

void TextEdit::SetOverlayScrollbar(bool overlay) {
  auto* scroll = static_cast<NSScrollView*>(GetNative());
  scroll.scrollerStyle = overlay ? NSScrollerStyleOverlay
                                 : NSScrollerStyleLegacy;
}

void TextEdit::SetScrollbarPolicy(Scroll::Policy h_policy,
                                  Scroll::Policy v_policy) {
  auto* scroll = static_cast<NSScrollView*>(GetNative());
  scroll.hasHorizontalScroller = h_policy != Scroll::Policy::Never;
  scroll.hasVerticalScroller = v_policy != Scroll::Policy::Never;
}

void TextEdit::SetScrollElasticity(Scroll::Elasticity h, Scroll::Elasticity v) {
  auto* scroll = static_cast<NSScrollView*>(GetNative());
  scroll.horizontalScrollElasticity = static_cast<NSScrollElasticity>(h);
  scroll.verticalScrollElasticity = static_cast<NSScrollElasticity>(v);
}

std::tuple<Scroll::Elasticity, Scroll::Elasticity> TextEdit::GetScrollElasticity() const {
  auto* scroll = static_cast<NSScrollView*>(GetNative());
  Scroll::Elasticity h = static_cast<Scroll::Elasticity>(scroll.horizontalScrollElasticity);
  Scroll::Elasticity v = static_cast<Scroll::Elasticity>(scroll.verticalScrollElasticity);
  return std::make_tuple(h, v);
}

RectF TextEdit::GetTextBounds() const {
  auto* textView = static_cast<NSTextView*>(
      [static_cast<NUTextEdit*>(GetNative()) documentView]);
  [textView.layoutManager ensureLayoutForTextContainer:textView.textContainer];
  return RectF([textView.layoutManager
      usedRectForTextContainer:textView.textContainer]);
}

}  // namespace nu
