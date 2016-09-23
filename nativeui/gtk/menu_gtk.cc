// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu.h"

namespace nu {

Menu::Menu() : MenuBase(GTK_MENU_SHELL(gtk_menu_new())) {
}

void Menu::Popup() {
  gtk_menu_popup(GTK_MENU(menu()), nullptr, nullptr, nullptr, nullptr,
                 0, gtk_get_current_event_time());
}

}  // namespace nu
