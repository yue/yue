// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_base.h"

#include <gtk/gtk.h>

#include "nativeui/menu_item.h"

namespace nu {

namespace {

// Find out the radio items that in the same group with |radio|.
GtkRadioMenuItem* SearchRadioInSameGroup(MenuBase* menu, MenuItem* radio,
                                         int index) {
  // First search backward.
  for (int i = index; i >= 0; --i) {
    nu::MenuItem* item = menu->ItemAt(i);
    if (item->GetType() == nu::MenuItem::Type::Separator)  // meet boundry
      break;
    else if (item != radio &&
             item->GetType() == nu::MenuItem::Type::Radio)  // found a radio
      return GTK_RADIO_MENU_ITEM(item->GetNative());
  }
  // Then search forward.
  for (int i = index + 1; i < menu->ItemCount(); ++i) {
    nu::MenuItem* item = menu->ItemAt(i);
    if (item->GetType() == nu::MenuItem::Type::Separator)  // meet boundry
      break;
    else if (item != radio &&
             item->GetType() == nu::MenuItem::Type::Radio)  // found a radio
      return GTK_RADIO_MENU_ITEM(item->GetNative());
  }
  return nullptr;
}

}  // namespace

void MenuBase::PlatformInit() {
  gtk_widget_show(GTK_WIDGET(menu_));
  g_object_ref_sink(menu_);
}

void MenuBase::PlatformDestroy() {
  gtk_widget_destroy(GTK_WIDGET(menu_));
  g_object_unref(menu_);
}

void MenuBase::PlatformInsert(MenuItem* item, int index) {
  if (GTK_IS_RADIO_MENU_ITEM(item->GetNative())) {
    GtkRadioMenuItem* group_item = SearchRadioInSameGroup(this, item, index);
    if (group_item)
      gtk_radio_menu_item_set_group(GTK_RADIO_MENU_ITEM(item->GetNative()),
                                    gtk_radio_menu_item_get_group(group_item));
  }
  gtk_menu_shell_insert(menu_, GTK_WIDGET(item->GetNative()), index);
}

void MenuBase::PlatformRemove(MenuItem* item) {
  gtk_container_remove(GTK_CONTAINER(menu_), GTK_WIDGET(item->GetNative()));
}

}  // namespace nu
