// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/menu_item_win.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/menu.h"
#include "nativeui/menu_item.h"
#include "nativeui/state.h"

namespace nu {

void MenuItem::Click() {
  if (type_ == nu::MenuItem::CheckBox)
    SetChecked(!IsChecked());
  else if (type_ == nu::MenuItem::Radio)
    SetChecked(true);
  on_click.Emit();
}

void MenuItem::SetLabel(const std::string& label) {
  menu_item_->label = label;
  menu_item_->actual_label = base::UTF8ToUTF16(label);
  if (!menu_item_->accelerator.empty()) {
    menu_item_->actual_label += L'\t';
    menu_item_->actual_label += menu_item_->accelerator;
  }
  if (menu_) {
    MENUITEMINFO mii = {0};
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STRING;
    mii.dwTypeData = const_cast<wchar_t*>(menu_item_->actual_label.c_str());
    mii.cch = static_cast<UINT>(menu_item_->actual_label.size());
    SetMenuItemInfo(menu_->menu(), menu_item_->id, FALSE, &mii);
  }
}

std::string MenuItem::GetLabel() const {
  return menu_item_->label;
}

void MenuItem::SetChecked(bool checked) {
  menu_item_->checked = checked;
  if (menu_) {
    if (checked && type_ == nu::MenuItem::Radio)
      FlipRadioMenuItems(menu_, this);
    UINT flags = MF_BYCOMMAND;
    flags |= checked ? MF_CHECKED : MF_UNCHECKED;
    CheckMenuItem(menu_->menu(), menu_item_->id, flags);
  }
}

bool MenuItem::IsChecked() const {
  return menu_item_->checked;
}

void MenuItem::SetEnabled(bool enabled) {
  menu_item_->enabled = enabled;
  if (menu_) {
    UINT flags = MF_BYCOMMAND;
    flags |= enabled ? MF_ENABLED : MF_DISABLED;
    EnableMenuItem(menu_->menu(), menu_item_->id, flags);
  }
}

bool MenuItem::IsEnabled() const {
  return menu_item_->enabled;
}

void MenuItem::SetVisible(bool visible) {
  menu_item_->visible = visible;
}

bool MenuItem::IsVisible() const {
  return menu_item_->visible;
}

void MenuItem::PlatformInit() {
  menu_item_ = new MenuItemData;
  menu_item_->id = State::current()->GetNextCommandID();
}

void MenuItem::PlatformDestroy() {
  delete menu_item_;
}

void MenuItem::PlatformSetSubmenu(Menu* submenu) {
  if (menu_) {
    MENUITEMINFO mii = {0};
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_SUBMENU;
    mii.hSubMenu = submenu_->menu();
    SetMenuItemInfo(menu_->menu(), menu_item_->id, FALSE, &mii);
  }
}

}  // namespace nu
