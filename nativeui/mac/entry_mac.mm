// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/entry.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"

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
  shell_->on_activate.Emit();
}

- (void)controlTextDidChange:(NSNotification*)notification {
  shell_->on_text_change.Emit();
}

@end

namespace nu {

Entry::Entry() {
  NSTextField* entry = [[NSTextField alloc] init];
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
  NSTextField* entry = static_cast<NSTextField*>(view());
  [entry.target release];
  [entry setTarget:nil];
  [entry setDelegate:nil];
}

void Entry::SetText(const std::string& text) {
  auto* entry = static_cast<NSTextField*>(view());
  [entry setStringValue:base::SysUTF8ToNSString(text)];
}

std::string Entry::GetText() const {
  auto* entry = static_cast<NSTextField*>(view());
  return base::SysNSStringToUTF8([entry stringValue]);
}

void Entry::SetBackgroundColor(Color color) {
  auto* entry = static_cast<NSTextField*>(view());
  [entry setBackgroundColor:color.ToNSColor()];
}

}  // namespace nu
