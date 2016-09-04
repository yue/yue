// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/button.h"

#import <Cocoa/Cocoa.h>

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

Button::Button(const std::string& title, Type type) {
  NSButton* button = [[NSButton alloc] init];
  if (type == Normal)
    [button setBezelStyle:NSSmallSquareBezelStyle];
  else if (type == CheckBox)
    [button setButtonType:NSSwitchButton];
  else if (type == Radio)
    [button setButtonType:NSRadioButton];

  [button setTarget:[[NUButtonDelegate alloc] initWithShell:this]];
  [button setAction:@selector(onClick:)];
  TakeOverView(button);

  SetTitle(title);
}

Button::~Button() {
  NSButton* button = static_cast<NSButton*>(view());
  [button.target release];
  [button setTarget:nil];
}

void Button::SetTitle(const std::string& title) {
  NSButton* button = static_cast<NSButton*>(view());
  button.title = base::SysUTF8ToNSString(title);

  // Calculate the preferred size by creating a new button.
  SetPreferredSize(Size([[button cell] cellSize]));
}

std::string Button::GetTitle() const {
  return base::SysNSStringToUTF8(static_cast<NSButton*>(view()).title);
}

void Button::SetChecked(bool checked) {
  static_cast<NSButton*>(view()).state = checked ? NSOnState : NSOffState;
}

bool Button::IsChecked() const {
  return static_cast<NSButton*>(view()).state == NSOnState;
}

}  // namespace nu
