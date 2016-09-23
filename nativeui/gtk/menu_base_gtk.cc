// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_base.h"

namespace nu {

MenuBase::~MenuBase() {
  gtk_widget_destroy(GTK_WIDGET(menu_));
  g_object_unref(menu_);
}

void MenuBase::PlatformInit() {
  gtk_widget_show(GTK_WIDGET(menu_));
  g_object_ref_sink(menu_);
}

}  // namespace nu
