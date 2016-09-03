// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"

namespace nu {

void Group::PlatformInit() {
  TakeOverView([[NSBox alloc] init]);
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

}  // namespace nu
