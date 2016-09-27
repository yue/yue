// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu.h"

#include "nativeui/menu_item.h"
#include "nativeui/state.h"
#include "nativeui/win/menu_item_win.h"

namespace nu {

Menu::Menu() : MenuBase(CreatePopupMenu()) {
}

void Menu::Popup() {
  POINT p;
  GetCursorPos(&p);
  UINT id = TrackPopupMenuEx(menu(),
                             TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                             p.x, p.y, State::current()->GetSubwinHolder(),
                             nullptr);
  if (id > 0) {
    // Find the item with the id and click it.
    for (int i = 0; i < item_count(); ++i) {
      nu::MenuItem* item = item_at(i);
      if (item->menu_item()->id == id) {
        item->Click();
        break;
      }
    }
  }
}

}  // namespace nu
