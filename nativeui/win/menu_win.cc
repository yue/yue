// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu.h"

#include "nativeui/menu_item.h"
#include "nativeui/state.h"
#include "nativeui/win/menu_base_win.h"
#include "nativeui/win/menu_item_win.h"

namespace nu {

Menu::Menu() : MenuBase(CreatePopupMenu()) {
}

void Menu::Popup() {
  POINT p;
  GetCursorPos(&p);
  OnMenuShow();
  UINT id = TrackPopupMenuEx(GetNative(),
                             TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                             p.x, p.y, State::GetCurrent()->GetSubwinHolder(),
                             nullptr);
  OnMenuHide();
  if (id > 0)
    DispatchCommandToItem(this, id);
}

}  // namespace nu
