// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_bar.h"

#include <gtk/gtk.h>

namespace nu {

NativeMenu MenuBar::PlatformCreate() const {
  GtkWidget* menu = gtk_menu_bar_new();
  // Make accelerators work even when menu bar is hidden.
  g_signal_connect(menu, "can_activate_accel", G_CALLBACK(gtk_true), nullptr);
  return GTK_MENU_SHELL(menu);
}

}  // namespace nu
