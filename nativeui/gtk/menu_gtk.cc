// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu.h"

#include <gtk/gtk.h>

#include "base/run_loop.h"

namespace nu {

namespace {

void OnMenuHidden(GtkWidget* widget, base::RunLoop* loop) {
  loop->Quit();
}

}  // namespace

Menu::Menu() : MenuBase(GTK_MENU_SHELL(gtk_menu_new())) {
}

void Menu::Popup() {
  gtk_menu_popup(GTK_MENU(menu()), nullptr, nullptr, nullptr, nullptr,
                 0, gtk_get_current_event_time());

  // Block until the menu is hidden.
  base::RunLoop loop;
  gint id = g_signal_connect(menu(), "hide", G_CALLBACK(OnMenuHidden), &loop);
  loop.Run();
  g_signal_handler_disconnect(menu(), id);
}

}  // namespace nu
