// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_item.h"

namespace nu {

void MenuItem::PlatformInit() {
  GtkWidget* item;
  switch (type_) {
    case Label: item = gtk_menu_item_new(); break;
    case Radio: item = gtk_radio_menu_item_new(nullptr); break;
    case CheckBox: item = gtk_check_menu_item_new(); break;
    case Separator: item = gtk_separator_menu_item_new(); break;
  }
  gtk_widget_show(item);
  g_object_ref_sink(item);
  menu_item_ = GTK_MENU_ITEM(item);
}

void MenuItem::PlatformDestroy() {
  gtk_widget_destroy(GTK_WIDGET(menu_item_));
  g_object_unref(menu_item_);
}

}  // namespace nu
