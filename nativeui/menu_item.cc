// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_item.h"

#include "nativeui/menu_base.h"

namespace nu {

MenuItem::MenuItem(Type type) : type_(type) {
  PlatformInit();
}

MenuItem::~MenuItem() {
  PlatformDestroy();
}

void MenuItem::SetSubmenu(MenuBase* submenu) {
  submenu_ = submenu;
  PlatformSetSubmenu(submenu);
}

MenuBase* MenuItem::GetSubmenu() const {
  return submenu_.get();
}

// Flip all radio items in the same group with |item|.
void MenuItem::FlipRadioMenuItems(nu::MenuBase* menu, nu::MenuItem* sender) {
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

}  // namespace nu
