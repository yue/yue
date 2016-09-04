// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

namespace nu {

void Group::PlatformInit() {
  TakeOverView(gtk_frame_new(""));

  // Give the frame an initial size to calculate border size.
  SetBounds(Rect(0, 0, 100, 100));
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

Size Group::GetBorderPixelSize() const {
  GdkRectangle outer;
  gtk_widget_get_allocation(view(), &outer);
  GdkRectangle inner;
  GTK_FRAME_GET_CLASS(view())->compute_child_allocation(
      GTK_FRAME(view()), &inner);
  return Size(outer.width - inner.width, outer.height - inner.height);
}

}  // namespace nu
