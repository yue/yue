// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/entry.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"

namespace nu {

Entry::Entry() {
  NSTextField* entry = [[NSTextField alloc] init];
  [entry setBordered:YES];
  [entry setBezelStyle:NSTextFieldSquareBezel];
  [entry setBezeled:YES];
  TakeOverView(entry);

  SetPreferredSize(Size(100, [[entry cell] cellSize].height));
}

Entry::~Entry() {
}

void Entry::SetText(const std::string& text) {
  auto* entry = static_cast<NSTextField*>(view());
  [entry setStringValue:base::SysUTF8ToNSString(text)];
}

std::string Entry::GetText() const {
  auto* entry = static_cast<NSTextField*>(view());
  return base::SysNSStringToUTF8([entry stringValue]);
}

}  // namespace nu
