// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_base.h"

#include "nativeui/menu_item.h"
#include "nativeui/win/menu_item_win.h"

namespace nu {

void MenuBase::PlatformInit() {
}

void MenuBase::PlatformDestroy() {
  DestroyMenu(menu_);
}

void MenuBase::PlatformInsert(MenuItem* item, int index) {
  auto* data = item->menu_item();
  if (!data->visible)
    return;

  MENUITEMINFO mii;
  mii.cbSize = sizeof(mii);
  if (item->type() == MenuItem::Separator) {
    mii.fMask = MIIM_FTYPE;
    mii.fType = MFT_SEPARATOR;
  } else {
    mii.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING | MIIM_STATE;
    mii.wID = data->id;
    if (item->type() == MenuItem::Radio)
      mii.fType = MFT_RADIOCHECK;
    else
      mii.fType = MFT_STRING;
    mii.fState = data->enabled ? MFS_ENABLED : MFS_ENABLED;
    if (data->checked)
      mii.fState = MFS_CHECKED;
    mii.dwTypeData = const_cast<wchar_t*>(data->label.c_str());
    mii.cch = static_cast<UINT>(data->label.size());
  }
  InsertMenuItem(menu_, index, TRUE, &mii);
}

void MenuBase::PlatformRemove(MenuItem* item) {
  RemoveMenu(menu_, item->menu_item()->id, MF_BYCOMMAND);
}

}  // namespace nu
