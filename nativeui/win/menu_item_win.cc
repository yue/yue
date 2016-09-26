// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/menu_item_win.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/menu_base.h"
#include "nativeui/menu_item.h"
#include "nativeui/state.h"

namespace nu {

void MenuItem::Click() {
}

void MenuItem::SetLabel(const std::string& label) {
  menu_item_->label = base::UTF8ToUTF16(label);
}

std::string MenuItem::GetLabel() const {
  return base::UTF16ToUTF8(menu_item_->label);
}

void MenuItem::SetChecked(bool checked) {
  menu_item_->checked = checked;
}

bool MenuItem::IsChecked() const {
  return menu_item_->checked;
}

void MenuItem::SetEnabled(bool enabled) {
  menu_item_->enabled = enabled;
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

void MenuItem::PlatformSetSubmenu(MenuBase* submenu) {
}

}  // namespace nu
