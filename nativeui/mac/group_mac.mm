// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"

namespace nu {

void Group::PlatformInit() {
  // Give the box an initial size to calculate border size.
  NSBox* group = [[NSBox alloc] init];
  [group setContentViewMargins:NSZeroSize];
  [group sizeToFit];
  TakeOverView(group);
}

void Group::PlatformSetContentView(View* view) {
  [static_cast<NSBox*>(GetNative()) setContentView:view->GetNative()];
  view->Layout();
}

void Group::SetTitle(const std::string& title) {
  static_cast<NSBox*>(GetNative()).title = base::SysUTF8ToNSString(title);
}

std::string Group::GetTitle() const {
  return base::SysNSStringToUTF8(static_cast<NSBox*>(GetNative()).title);
}

SizeF Group::GetBorderSize() const {
  NSSize outer = [GetNative() frame].size;
  NSSize inner = [GetContentView()->GetNative() frame].size;
  return SizeF(outer.width - inner.width, outer.height - inner.height);
}

}  // namespace nu
