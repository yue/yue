// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_item.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"
#include "nativeui/menu_base.h"

namespace nu {

void MenuItem::SetLabel(const std::string& label) {
  [menu_item_ setTitle:base::SysUTF8ToNSString(label)];
}

std::string MenuItem::GetLabel() const {
  return base::SysNSStringToUTF8([menu_item_ title]);
}

void MenuItem::PlatformInit() {
  menu_item_ = [[NSMenuItem alloc] init];
}

void MenuItem::PlatformDestroy() {
  [menu_item_ release];
}

void MenuItem::PlatformSetSubmenu(MenuBase* submenu) {
  [menu_item_ setSubmenu:submenu->menu()];
}

}  // namespace nu
