// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"

namespace nu {

Label::Label() {
  NSTextField* label =
      [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
  label.drawsBackground = NO;
  label.bordered = NO;
  label.editable = NO;
  label.selectable = NO;
  set_view(label);
}

Label::~Label() {
}

void Label::SetText(const std::string& text) {
  static_cast<NSTextField*>(view()).stringValue = base::SysUTF8ToNSString(text);
}

std::string Label::GetText() {
  return base::SysNSStringToUTF8(static_cast<NSTextField*>(view()).stringValue);
}

}  // namespace nu
