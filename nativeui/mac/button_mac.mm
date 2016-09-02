// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/button.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"

@interface NUButtonDelegate : NSObject {
 @private
  nu::Button* shell_;
}
- (id)initWithShell:(nu::Button*)shell;
- (IBAction)onClick:(id)sender;
@end

@implementation NUButtonDelegate

- (id)initWithShell:(nu::Button*)shell {
  if ((self = [super init]))
    shell_ = shell;
  return self;
}

- (IBAction)onClick:(id)sender {
  shell_->on_click.Emit();
}

@end

namespace nu {

Button::Button(const std::string& title) {
  NSButton* button = [[NSButton alloc] init];
  button.bezelStyle = NSRoundedBezelStyle;
  button.target = [[NUButtonDelegate alloc] initWithShell:this];
  button.action = @selector(onClick:);
  set_view(button);

  SetTitle(title);
}

Button::~Button() {
  NSButton* button = static_cast<NSButton*>(view());
  [button.target release];
  button.target = nil;
}

void Button::SetTitle(const std::string& title) {
  NSButton* button = static_cast<NSButton*>(view());
  button.title = base::SysUTF8ToNSString(title);

  // Calculate the preferred size by creating a new button.
  base::scoped_nsobject<NSButton> tmp([[NSButton alloc] init]);
  [tmp setBezelStyle:button.bezelStyle];
  [tmp setTitle:button.title];
  [tmp sizeToFit];
  SetPreferredSize(Size([tmp frame].size));
}

std::string Button::GetTitle() const {
  return base::SysNSStringToUTF8(static_cast<NSButton*>(view()).title);
}

}  // namespace nu
