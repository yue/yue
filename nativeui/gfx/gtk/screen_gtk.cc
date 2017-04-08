// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/screen.h"

#include <gtk/gtk.h>

namespace nu {

namespace {

const float kDefaultDPI = 96;

}  // namespace

float GetScaleFactor() {
  GtkSettings* gtk_settings = gtk_settings_get_default();
  int gtk_dpi = -1;
  g_object_get(gtk_settings, "gtk-xft-dpi", &gtk_dpi, nullptr);

  // GTK multiplies the DPI by 1024 before storing it.
  int dpi = (gtk_dpi > 0) ? gtk_dpi / 1024.0 : kDefaultDPI;
  return dpi / kDefaultDPI;
}

}  // namespace nu
