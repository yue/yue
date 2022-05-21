// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/entry.h"

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/font.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUEntry : NSTextField<NUViewMethods> {
 @private
  nu::NUViewPrivate private_;
}
@end

@implementation NUEntry

- (nu::NUViewPrivate*)nuPrivate {
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

- (void)setNUEnabled:(BOOL)enabled {
  [self setEditable:enabled];
}

- (BOOL)isNUEnabled {
  return [self isEditable];
}

@end

// It is sad that with Object-C we have to repeat the code.
@interface NUSecureEntry : NSSecureTextField<NUViewMethods> {
 @private
  nu::NUViewPrivate private_;
}
@end

@implementation NUSecureEntry

- (nu::NUViewPrivate*)nuPrivate {
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

- (void)setNUEnabled:(BOOL)enabled {
  [self setEditable:enabled];
}

- (BOOL)isNUEnabled {
  return [self isEditable];
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

Entry::Entry(Type type) {
  NSTextField* entry = type == Type::Normal ? [[NUEntry alloc] init]
                                            : [[NUSecureEntry alloc] init];
  [entry setBezelStyle:NSTextFieldSquareBezel];
  [entry setBezeled:YES];
  [entry setTarget:[[NUEntryDelegate alloc] initWithShell:this]];
  [entry setAction:@selector(onActivate:)];
  [entry setDelegate:entry.target];
  TakeOverView(entry);
  UpdateDefaultStyle();
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

SizeF Entry::GetMinimumSize() const {
  auto* entry = static_cast<NSTextField*>(GetNative());
  return SizeF(0, [[entry cell] cellSize].height);
}

}  // namespace nu
