// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_MENU_BASE_WIN_H_
#define NATIVEUI_WIN_MENU_BASE_WIN_H_

#include "nativeui/menu_base.h"

namespace nu {

// Iterate the menu recursively to find the item with command id, and then emit
// the click event for it.
void DispatchCommandToItem(nu::MenuBase* menu, int command);

}  // namespace nu

#endif  // NATIVEUI_WIN_MENU_BASE_WIN_H_
