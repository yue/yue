// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/menu_base_win.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/menu.h"
#include "nativeui/menu_item.h"
#include "nativeui/win/menu_item_win.h"

namespace nu {

void DispatchCommandToItem(MenuBase* menu, int command) {
  // Find the item with the id and click it.
  for (int i = 0; i < menu->ItemCount(); ++i) {
    MenuItem* item = menu->ItemAt(i);
    if (item->GetNative()->id == command) {
      item->Click();
      break;
    } else if (item->GetType() == MenuItem::Type::Submenu &&
               item->GetSubmenu()) {
      DispatchCommandToItem(item->GetSubmenu(), command);
    }
  }
}

void MenuBase::PlatformInit() {
}

void MenuBase::PlatformDestroy() {
  DestroyMenu(menu_);
}

void MenuBase::PlatformInsert(MenuItem* item, int index) {
  MENUITEMINFO mii = {0};
  mii.cbSize = sizeof(mii);
  if (item->GetType() == MenuItem::Type::Separator) {
    mii.fMask = MIIM_FTYPE;
    mii.fType = MFT_SEPARATOR;
  } else {
    auto* data = item->GetNative();
    mii.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING | MIIM_STATE;
    mii.wID = data->id;
    if (item->GetType() == MenuItem::Type::Submenu && item->GetSubmenu()) {
      mii.fMask |= MIIM_SUBMENU;
      mii.hSubMenu = item->GetSubmenu()->GetNative();
    }
    if (item->GetImage()) {
      mii.fMask |= MIIM_BITMAP;
      mii.hbmpItem = item->bitmap_image_.get();
    }
    if (item->GetType() == MenuItem::Type::Radio)
      mii.fType = MFT_RADIOCHECK;
    else
      mii.fType = MFT_STRING;
    mii.fState = data->enabled ? MFS_ENABLED : MFS_ENABLED;
    if (data->checked)
      mii.fState = MFS_CHECKED;
    mii.dwTypeData = const_cast<wchar_t*>(data->actual_label.c_str());
    mii.cch = static_cast<UINT>(data->actual_label.size());
  }
  InsertMenuItem(menu_, index, TRUE, &mii);
}

void MenuBase::PlatformRemove(MenuItem* item) {
  RemoveMenu(menu_, item->GetNative()->id, MF_BYCOMMAND);
}

}  // namespace nu
