// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_item.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"
#include "nativeui/menu_base.h"

namespace {

// Flip all radio items in the same group with |item|.
void FlipRadioMenuItems(nu::MenuBase* menu, nu::MenuItem* sender) {
  // Find out from where the group starts.
  int group_start = 0;
  int radio_count = 0;
  bool found_item = false;
  for (int i = 0; i < menu->item_count(); ++i) {
    nu::MenuItem* item = menu->item_at(i);
    if (item == sender) {
      found_item = true;  // in the group now
    } else if (item->type() == nu::MenuItem::Separator) {
      if (found_item)  // end of group
        break;
      // Possible start of a the group.
      radio_count = 0;
      group_start = i;
    } else if (item->type() == nu::MenuItem::Radio) {
      radio_count++;  // another radio in the group
    }
  }

  // No need to flip if there is only one radio in group.
  if (radio_count == 0)
    return;

  // Flip all other radios in the group.
  for (int i = group_start; i < menu->item_count(); ++i) {
    nu::MenuItem* item = menu->item_at(i);
    if (item != sender && item->type() == nu::MenuItem::Radio)
      item->SetChecked(false);
  }
}

}  // namespace

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
  if (shell_->type() == nu::MenuItem::CheckBox) {
    shell_->SetChecked(!shell_->IsChecked());
  } else if (shell_->type() == nu::MenuItem::Radio) {
    shell_->SetChecked(true);
    FlipRadioMenuItems(shell_->menu(), shell_);
  }
  shell_->on_click.Emit();
}

@end

namespace nu {

void MenuItem::SetLabel(const std::string& label) {
  menu_item_.title = base::SysUTF8ToNSString(label);
}

std::string MenuItem::GetLabel() const {
  return base::SysNSStringToUTF8(menu_item_.title);
}

void MenuItem::SetChecked(bool checked) {
  menu_item_.state = checked ? NSOnState : NSOffState;
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

void MenuItem::PlatformSetSubmenu(MenuBase* submenu) {
  [menu_item_ setSubmenu:submenu->menu()];
}

}  // namespace nu
