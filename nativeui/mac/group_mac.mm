// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"

namespace nu {

void Group::PlatformInit() {
  // Give the box an initial size to calculate border size.
  TakeOverView([[NSBox alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)]);
}

void Group::PlatformSetContentView(Container* container) {
  [static_cast<NSBox*>(view()) setContentView:container->view()];
  container->Layout();
}

void Group::SetTitle(const std::string& title) {
  static_cast<NSBox*>(view()).title = base::SysUTF8ToNSString(title);
}

std::string Group::GetTitle() const {
  return base::SysNSStringToUTF8(static_cast<NSBox*>(view()).title);
}

Size Group::GetBorderPixelSize() const {
  NSSize outer = [view() frame].size;
  NSSize inner = [GetContentView()->view() frame].size;
  return Size(outer.width - inner.width, outer.height - inner.height);
}

}  // namespace nu
