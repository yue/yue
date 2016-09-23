// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_base.h"

#include "nativeui/menu_item.h"

namespace nu {

void MenuBase::PlatformInit() {
  gtk_widget_show(GTK_WIDGET(menu_));
  g_object_ref_sink(menu_);
}

void MenuBase::PlatformDestroy() {
  gtk_widget_destroy(GTK_WIDGET(menu_));
  g_object_unref(menu_);
}

void MenuBase::PlatformInsert(MenuItem* item, int index) {
  gtk_menu_shell_insert(menu_, GTK_WIDGET(item->menu_item()), index);
}

void MenuBase::PlatformRemove(MenuItem* item) {
  gtk_container_remove(GTK_CONTAINER(menu_), GTK_WIDGET(item->menu_item()));
}

}  // namespace nu
