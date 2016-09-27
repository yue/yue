// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_MENU_ITEM_WIN_H_
#define NATIVEUI_WIN_MENU_ITEM_WIN_H_

#include <string>

#include <windows.h>

namespace nu {

struct MenuItemData {
  UINT id = 0;
  std::wstring label;
  bool checked = false;
  bool enabled = true;
  bool visible = true;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_MENU_ITEM_WIN_H_
