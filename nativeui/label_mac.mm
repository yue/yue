// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#import <Cocoa/Cocoa.h>

#include "nativeui/scoped_types_mac.h"

namespace nu {

Label::Label() : View(nullptr) {
  NSTextField* label =
      [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
  label.drawsBackground = NO;
  label.bordered = NO;
  label.editable = NO;
  label.selectable = NO;
  label.stringValue = @"test";
  view_.Reset(label);
}

Label::~Label() {
}

}  // namespace nu
