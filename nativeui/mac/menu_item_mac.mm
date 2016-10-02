// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_item.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"
#include "nativeui/menu.h"

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
  if (shell_->type() == nu::MenuItem::CheckBox)
    shell_->SetChecked(!shell_->IsChecked());
  else if (shell_->type() == nu::MenuItem::Radio)
    shell_->SetChecked(true);
  shell_->on_click.Emit();
}

@end

namespace nu {

void MenuItem::Click() {
  [NSApp sendAction:menu_item_.action to:menu_item_.target from:menu_item_];
}

void MenuItem::SetLabel(const std::string& label) {
  menu_item_.title = base::SysUTF8ToNSString(label);
}

std::string MenuItem::GetLabel() const {
  return base::SysNSStringToUTF8(menu_item_.title);
}

void MenuItem::SetChecked(bool checked) {
  menu_item_.state = checked ? NSOnState : NSOffState;
  if (checked && type_ == nu::MenuItem::Radio && menu_)
    FlipRadioMenuItems(menu_, this);
}

bool MenuItem::IsChecked() const {
  return menu_item_.state == NSOnState;
}

void MenuItem::SetEnabled(bool enabled) {
  menu_item_.enabled = enabled;
}

bool MenuItem::IsEnabled() const {
  return menu_item_.enabled;
}

void MenuItem::SetVisible(bool visible) {
  menu_item_.hidden = !visible;
}

bool MenuItem::IsVisible() const {
  return !menu_item_.hidden;
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

void MenuItem::PlatformSetSubmenu(Menu* submenu) {
  [menu_item_ setSubmenu:submenu->menu()];
}

}  // namespace nu
