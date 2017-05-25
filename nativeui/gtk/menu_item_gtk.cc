// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_item.h"

#include <gtk/gtk.h>

#include "nativeui/menu.h"

namespace nu {

namespace {

void OnClick(GtkMenuItem*, MenuItem* item) {
  item->on_click.Emit(item);
}

}  // namespace

void MenuItem::Click() {
  gtk_menu_item_activate(menu_item_);
}

void MenuItem::SetLabel(const std::string& label) {
  gtk_menu_item_set_label(menu_item_, label.c_str());
}

std::string MenuItem::GetLabel() const {
  return gtk_menu_item_get_label(menu_item_);
}

void MenuItem::SetChecked(bool checked) {
  if (GTK_IS_CHECK_MENU_ITEM(menu_item_))
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item_), checked);
}

bool MenuItem::IsChecked() const {
  if (GTK_IS_CHECK_MENU_ITEM(menu_item_))
    return gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu_item_));
  return false;
}

void MenuItem::SetEnabled(bool enabled) {
  gtk_widget_set_sensitive(GTK_WIDGET(menu_item_), enabled);
}

bool MenuItem::IsEnabled() const {
  return gtk_widget_get_sensitive(GTK_WIDGET(menu_item_));
}

void MenuItem::SetVisible(bool visible) {
  gtk_widget_set_visible(GTK_WIDGET(menu_item_), visible);
}

bool MenuItem::IsVisible() const {
  return gtk_widget_get_visible(GTK_WIDGET(menu_item_));
}

void MenuItem::PlatformInit() {
  GtkWidget* item;
  switch (type_) {
    case Type::Label: case Type::Submenu: item = gtk_menu_item_new(); break;
    case Type::Radio: item = gtk_radio_menu_item_new(nullptr); break;
    case Type::Checkbox: item = gtk_check_menu_item_new(); break;
    case Type::Separator: item = gtk_separator_menu_item_new(); break;
  }
  g_signal_connect(item, "activate", G_CALLBACK(OnClick), this);

  gtk_widget_show(item);
  g_object_ref_sink(item);
  menu_item_ = GTK_MENU_ITEM(item);
}

void MenuItem::PlatformDestroy() {
  gtk_widget_destroy(GTK_WIDGET(menu_item_));
  g_object_unref(menu_item_);
}

void MenuItem::PlatformSetSubmenu(Menu* submenu) {
  gtk_menu_item_set_submenu(menu_item_, GTK_WIDGET(submenu->GetNative()));
}

}  // namespace nu
