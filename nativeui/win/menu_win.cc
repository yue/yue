// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu.h"

#include "nativeui/state.h"

namespace nu {

Menu::Menu() : MenuBase(CreatePopupMenu()) {
}

void Menu::Popup() {
  POINT p;
  GetCursorPos(&p);
  TrackPopupMenuEx(menu(), TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                   p.x, p.y, State::current()->GetSubwinHolder(), nullptr);
}

}  // namespace nu
