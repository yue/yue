// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_item.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"
#include "nativeui/menu_base.h"

@interface NUMenuItemDelegate : NSObject {
 @private
  nu::MenuItem* shell_;
}
- (id)initWithShell:(nu::MenuItem*)shell;
- (IBAction)onClick:(id)sender;
@end

@implementation NUMenuItemDelegate

- (id)initWithShell:(nu::MenuItem*)shell {
  if ((self = [super init]))
    shell_ = shell;
  return self;
}

- (IBAction)onClick:(id)sender {
  shell_->on_click.Emit();
}

@end

namespace nu {

void MenuItem::SetLabel(const std::string& label) {
  [menu_item_ setTitle:base::SysUTF8ToNSString(label)];
}

std::string MenuItem::GetLabel() const {
  return base::SysNSStringToUTF8([menu_item_ title]);
}

void MenuItem::PlatformInit() {
  if (type_ == Separator)
    menu_item_ = [[NSMenuItem separatorItem] retain];
  else
    menu_item_ = [[NSMenuItem alloc] init];
  menu_item_.target = [[NUMenuItemDelegate alloc] initWithShell:this];
  menu_item_.action = @selector(onClick:);
}

void MenuItem::PlatformDestroy() {
  [menu_item_.target release];
  [menu_item_ release];
}

void MenuItem::PlatformSetSubmenu(MenuBase* submenu) {
  [menu_item_ setSubmenu:submenu->menu()];
}

}  // namespace nu
