// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#import <Cocoa/Cocoa.h>

#include "nativeui/scoped_types_mac.h"

namespace nu {

View::View() {
  view_.Reset([[NSView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)]);
}

View::View(nullptr_t) {
}

View::~View() {
}

}  // namespace nu
