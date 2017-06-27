// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu.h"

#include <gtk/gtk.h>

namespace nu {

namespace {

void OnMenuHidden(GtkWidget* widget, void*) {
  gtk_main_quit();
}

}  // namespace

Menu::Menu() : MenuBase(GTK_MENU_SHELL(gtk_menu_new())) {
}

void Menu::Popup() {
  gtk_menu_popup(GTK_MENU(GetNative()), nullptr, nullptr, nullptr, nullptr,
                 0, gtk_get_current_event_time());

  // Block until the menu is hidden.
  gint id = g_signal_connect(GetNative(), "hide", G_CALLBACK(OnMenuHidden),
                             nullptr);
  gtk_main();
  g_signal_handler_disconnect(GetNative(), id);
}

}  // namespace nu
