// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include <gtk/gtk.h>

#include "nativeui/gfx/attributed_text.h"
#include "nativeui/gtk/nu_label.h"

namespace nu {

NativeView Label::PlatformCreate() {
  GtkWidget* event_box = gtk_event_box_new();
  GtkWidget* label = nu_label_new(this);
  gtk_widget_show(label);
  gtk_container_add(GTK_CONTAINER(event_box), label);
  return event_box;
}

}  // namespace nu
