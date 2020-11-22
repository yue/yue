// Copyright 2020 Cheng Zhao. All rights reserved.
// Copyright 2013 GitHub, Inc.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/message_box.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/image.h"
#include "nativeui/window.h"

namespace nu {

namespace {

// Cocoa returns this ID when message box is cancelled.
const int kCancelId = -1000;

}  // namespace

MessageBox::MessageBox() {
  box_ = [[NSAlert alloc] init];
}

MessageBox::~MessageBox() {
  [box_ release];
}

int MessageBox::PlatformRun() {
  int res = [box_ runModal];
  return res == kCancelId ? cancel_response_ : res;
}

int MessageBox::PlatformRunForWindow(Window* window) {
  __block int res = cancel_response_;
  [box_ beginSheetModalForWindow:window->GetNative()
               completionHandler:^(NSModalResponse response) {
                 res = response;
                 [NSApp stopModal];
               }];
  [NSApp runModalForWindow:window->GetNative()];
  return res == kCancelId ? cancel_response_ : res;
}

void MessageBox::PlatformShowForWindow(Window* window) {
  __block scoped_refptr<MessageBox> ref = this;
  [box_ beginSheetModalForWindow:window->GetNative()
               completionHandler:^(NSModalResponse res) {
                 ref->OnClose(res == kCancelId ? ref->cancel_response_ : res);
               }];
}

void MessageBox::PlatformClose() {
  [NSApp endSheet:box_.window];
}

void MessageBox::SetType(Type type) {
  switch (type) {
    case Type::None:
    case Type::Information:
      box_.alertStyle = NSInformationalAlertStyle;
      break;
    case Type::Warning:
      box_.alertStyle = NSWarningAlertStyle;
      break;
    case Type::Error:
      box_.alertStyle = NSCriticalAlertStyle;
      break;
  }
}

void MessageBox::AddButton(const std::string& title, int response) {
  // Empty title causes crash on macOS.
  NSString* nt = title.empty() ? @"(empty)" : base::SysUTF8ToNSString(title);
  NSButton* button = [box_ addButtonWithTitle:nt];
  [button setTag:response];
  if (default_response_ && *default_response_ == response)
    [button setKeyEquivalent:@"\r"];
  else
    [button setKeyEquivalent:@""];
  if (cancel_response_ == response)
    [button setKeyEquivalent:@"\e"];
}

void MessageBox::PlatformSetDefaultResponse() {
  for (NSButton* button in [box_ buttons]) {
    bool is_default = button.tag == *default_response_;
    if (is_default)
      [button setKeyEquivalent:@"\r"];
    else if ([button.keyEquivalent isEqualToString:@"\r"])
      [button setKeyEquivalent:@""];
  }
}

void MessageBox::PlatformSetCancelResponse() {
  for (NSButton* button in [box_ buttons]) {
    bool is_cancel = button.tag == cancel_response_;
    if (is_cancel)
      [button setKeyEquivalent:@"\e"];
    else if ([button.keyEquivalent isEqualToString:@"\e"])
      [button setKeyEquivalent:@""];
  }
}

void MessageBox::SetText(const std::string& text) {
  [box_ setMessageText:base::SysUTF8ToNSString(text)];
}

void MessageBox::SetInformativeText(const std::string& text) {
  [box_ setInformativeText:base::SysUTF8ToNSString(text)];
}

void MessageBox::SetAccessoryView(scoped_refptr<View> view) {
  [box_ setAccessoryView:view->GetNative()];
  accessory_view_ = std::move(view);
}

void MessageBox::PlatformSetImage(Image* image) {
  [box_ setIcon:image ? image->GetNative() : nil];
}

}  // namespace nu
