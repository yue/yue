// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

namespace nu {

void Group::PlatformInit() {
  set_view(gtk_frame_new(""));
  g_object_ref_sink(view());
  gtk_widget_show(view());
}

void Group::PlatformSetContentView(Container* container) {
  GtkWidget* child = gtk_bin_get_child(GTK_BIN(view()));
  if (child)
    gtk_container_remove(GTK_CONTAINER(view()), child);
  gtk_container_add(GTK_CONTAINER(view()), container->view());

  container->Layout();
}

void Group::SetTitle(const std::string& title) {
  gtk_frame_set_label(GTK_FRAME(view()), title.c_str());
}

std::string Group::GetTitle() const {
  return gtk_frame_get_label(GTK_FRAME(view()));
}

}  // namespace nu

