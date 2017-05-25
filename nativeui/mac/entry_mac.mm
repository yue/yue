// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/entry.h"

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/font.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUEntry : NSTextField<NUView> {
 @private
  nu::NUPrivate private_;
}
- (nu::NUPrivate*)nuPrivate;
- (void)setNUFont:(nu::Font*)font;
- (void)setNUColor:(nu::Color)color;
- (void)setNUBackgroundColor:(nu::Color)color;
@end

@implementation NUEntry

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

@interface NUEntryDelegate : NSObject<NSTextFieldDelegate> {
 @private
  nu::Entry* shell_;
}
- (id)initWithShell:(nu::Entry*)shell;
- (IBAction)onActivate:(id)sender;
@end

@implementation NUEntryDelegate

- (id)initWithShell:(nu::Entry*)shell {
  if ((self = [super init]))
    shell_ = shell;
  return self;
}

- (IBAction)onActivate:(id)sender {
  shell_->on_activate.Emit(shell_);
}

- (void)controlTextDidChange:(NSNotification*)notification {
  shell_->on_text_change.Emit(shell_);
}

@end

namespace nu {

Entry::Entry() {
  NSTextField* entry = [[NUEntry alloc] init];
  [entry setBezelStyle:NSTextFieldSquareBezel];
  [entry setBezeled:YES];
  [entry setTarget:[[NUEntryDelegate alloc] initWithShell:this]];
  [entry setAction:@selector(onActivate:)];
  [entry setDelegate:entry.target];
  TakeOverView(entry);

  float height = [[entry cell] cellSize].height;
  SetDefaultStyle(SizeF(0, height));
}

Entry::~Entry() {
  NSTextField* entry = static_cast<NSTextField*>(GetNative());
  [entry.target release];
  [entry setTarget:nil];
  [entry setDelegate:nil];
}

void Entry::SetText(const std::string& text) {
  auto* entry = static_cast<NSTextField*>(GetNative());
  [entry setStringValue:base::SysUTF8ToNSString(text)];
}

std::string Entry::GetText() const {
  auto* entry = static_cast<NSTextField*>(GetNative());
  return base::SysNSStringToUTF8([entry stringValue]);
}

}  // namespace nu
