// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_item.h"

#include "nativeui/menu_base.h"

namespace nu {

MenuItem::MenuItem(Type type) : type_(type) {
  PlatformInit();
}

MenuItem::~MenuItem() {
  PlatformDestroy();
}

void MenuItem::SetSubmenu(MenuBase* submenu) {
  submenu_ = submenu;
  PlatformSetSubmenu(submenu);
}

MenuBase* MenuItem::GetSubmenu() const {
  return submenu_.get();
}

}  // namespace nu
