// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_item.h"

#include "nativeui/accelerator_manager.h"
#include "nativeui/menu.h"

namespace nu {

namespace {

MenuItem::Type RoleToType(MenuItem::Role role) {
  switch (role) {
    case MenuItem::Role::Edit:
    case MenuItem::Role::Help:
    case MenuItem::Role::Window:
    case MenuItem::Role::Services:
      return MenuItem::Type::Submenu;
    default:
      return MenuItem::Type::Label;
  }
}

}  // namespace

MenuItem::MenuItem(Type type) : type_(type) {
  PlatformInit();
}

MenuItem::MenuItem(Role role) : role_(role), type_(RoleToType(role)) {
  PlatformInit();
}

MenuItem::~MenuItem() {
  PlatformDestroy();
}

void MenuItem::SetSubmenu(Menu* submenu) {
  submenu_ = submenu;
  PlatformSetSubmenu(submenu);
}

Menu* MenuItem::GetSubmenu() const {
  return submenu_.get();
}

void MenuItem::SetAccelerator(const Accelerator& accelerator) {
  if (accel_manager_) {
    if (accelerator.IsEmpty())
      accel_manager_->RemoveAccelerator(this, accelerator_);
    else
      accel_manager_->RegisterAccelerator(this, accelerator);
  }
  accelerator_ = accelerator;
}

Accelerator MenuItem::GetAccelerator() const {
  return accelerator_;
}

// Flip all radio items in the same group with |item|.
void MenuItem::FlipRadioMenuItems(nu::MenuBase* menu, nu::MenuItem* sender) {
  // Find out from where the group starts.
  int group_start = 0;
  int radio_count = 0;
  bool found_item = false;
  for (int i = 0; i < menu->ItemCount(); ++i) {
    nu::MenuItem* item = menu->ItemAt(i);
    if (item == sender) {
      found_item = true;  // in the group now
    } else if (item->GetType() == Type::Separator) {
      if (found_item)  // end of group
        break;
      // Possible start of a the group.
      radio_count = 0;
      group_start = i;
    } else if (item->GetType() == Type::Radio) {
      radio_count++;  // another radio in the group
    }
  }

  // No need to flip if there is only one radio in group.
  if (radio_count == 0)
    return;

  // Flip all other radios in the group.
  for (int i = group_start; i < menu->ItemCount(); ++i) {
    nu::MenuItem* item = menu->ItemAt(i);
    if (item != sender && item->GetType() == Type::Radio)
      item->SetChecked(false);
  }
}

void MenuItem::SetAcceleratorManager(AcceleratorManager* accel_manager) {
  if (submenu_) {
    submenu_->SetAcceleratorManager(accel_manager);
    return;
  }

  if (accel_manager_ && !accelerator_.IsEmpty())
    accel_manager_->RemoveAccelerator(this, accelerator_);
  accel_manager_ = accel_manager;
  if (accel_manager_ && !accelerator_.IsEmpty())
    accel_manager_->RegisterAccelerator(this, accelerator_);
}

}  // namespace nu
