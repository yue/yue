// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_item.h"

#include <utility>

#include "nativeui/accelerator_manager.h"
#include "nativeui/gfx/image.h"
#include "nativeui/menu.h"

namespace nu {

namespace {

#if defined(OS_MAC)
#define CONTROL_OR_COMMAND MASK_META
#else
#define CONTROL_OR_COMMAND MASK_CONTROL
#endif

// Default mappings for MenuItem roles.
struct {
  const char* label;
  KeyboardCode key;
  int mask;
} g_roles_map[] = {
  { "Copy",         VKEY_C, CONTROL_OR_COMMAND },
  { "Cut",          VKEY_X, CONTROL_OR_COMMAND },
  { "Paste",        VKEY_V, CONTROL_OR_COMMAND },
  { "Select All",   VKEY_A, CONTROL_OR_COMMAND },
  { "Undo",         VKEY_Z, CONTROL_OR_COMMAND },
  { "Redo",         VKEY_Z, CONTROL_OR_COMMAND | MASK_SHIFT },
  { "Minimize",     VKEY_M, CONTROL_OR_COMMAND },
#if defined(OS_MAC)
  { "Zoom",         VKEY_UNKNOWN, 0 },
#else
  { "Maximize",     VKEY_UNKNOWN, 0 },
#endif
  { "Close Window", VKEY_W, CONTROL_OR_COMMAND },
#if defined(OS_MAC)
  { "About",        VKEY_UNKNOWN, 0 },
  { "Hide",         VKEY_H, CONTROL_OR_COMMAND },
  { "Hide Others",  VKEY_H, CONTROL_OR_COMMAND | MASK_ALT },
  { "Unhide",       VKEY_UNKNOWN, 0 },
#endif
};

static_assert(
    std::size(g_roles_map) == static_cast<size_t>(MenuItem::Role::ItemCount),
    "g_roles_map should be updated with roles");

// Get the MenuItem type from its role.
inline MenuItem::Type RoleToType(MenuItem::Role role) {
  if (role > MenuItem::Role::ItemCount && role != MenuItem::Role::None)
    return MenuItem::Type::Submenu;
  else
    return MenuItem::Type::Label;
}

}  // namespace

MenuItem::MenuItem(Type type) : type_(type) {
  PlatformInit();
}

MenuItem::MenuItem(Role role) : role_(role), type_(RoleToType(role)) {
  PlatformInit();
  // Some platforms do not have stock items, so fill with fallback settings.
  if (role_ < MenuItem::Role::ItemCount) {
    const auto& fallback = g_roles_map[static_cast<int>(role_)];
    if (GetLabel().empty())
      SetLabel(fallback.label);
    if (GetAccelerator().IsEmpty())
      SetAccelerator(Accelerator(fallback.key, fallback.mask));
  }
}

MenuItem::~MenuItem() {
  PlatformDestroy();
}

void MenuItem::SetSubmenu(scoped_refptr<Menu> submenu) {
  if (submenu_)
    submenu_->SetParent(nullptr);
  PlatformSetSubmenu(submenu.get());
  submenu_ = std::move(submenu);
  if (submenu_)
    submenu_->SetParent(this);
}

Menu* MenuItem::GetSubmenu() const {
  return submenu_.get();
}

#if defined(OS_MAC) || defined(OS_WIN)
void MenuItem::SetImage(scoped_refptr<Image> image) {
  PlatformSetImage(image.get());
  image_ = std::move(image);
}

Image* MenuItem::GetImage() const {
  return image_.get();
}
#endif

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

MenuBase* MenuItem::FindTopLevelMenu() const {
  MenuBase* menu = GetMenu();
  if (!menu)
    return nullptr;
  MenuItem* parent = menu->GetParent();
  while (parent) {
    menu = parent->GetMenu();
    if (!menu)
      return nullptr;
    parent = menu->GetParent();
  }
  return menu;
}

void MenuItem::EmitClick() {
#if !defined(OS_MAC)
  if (validate && !validate(this))
    return;
#endif
#if !defined(OS_LINUX)
  if (type_ == Type::Checkbox)
    SetChecked(!IsChecked());
  else if (type_ == Type::Radio)
    SetChecked(true);
#endif
  on_click.Emit(this);
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
