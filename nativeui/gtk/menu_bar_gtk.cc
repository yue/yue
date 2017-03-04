// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_bar.h"

#include <gtk/gtk.h>

namespace nu {

NativeMenu MenuBar::PlatformCreate() const {
  return GTK_MENU_SHELL(gtk_menu_bar_new());
}

}  // namespace nu
