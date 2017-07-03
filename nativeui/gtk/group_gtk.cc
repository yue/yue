// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

#include <gtk/gtk.h>

namespace nu {

void Group::PlatformInit() {
  TakeOverView(gtk_frame_new(""));

  // Give the frame an initial size to calculate border size.
  GdkRectangle rect = { 0, 0, 100, 100 };
  gtk_widget_size_allocate(GetNative(), &rect);
}

void Group::PlatformSetContentView(View* view) {
  GtkWidget* child = gtk_bin_get_child(GTK_BIN(GetNative()));
  if (child)
    gtk_container_remove(GTK_CONTAINER(GetNative()), child);
  gtk_container_add(GTK_CONTAINER(GetNative()), view->GetNative());

  view->Layout();
}

void Group::SetTitle(const std::string& title) {
  gtk_frame_set_label(GTK_FRAME(GetNative()), title.c_str());
}

std::string Group::GetTitle() const {
  return gtk_frame_get_label(GTK_FRAME(GetNative()));
}

SizeF Group::GetBorderSize() const {
  GdkRectangle outer;
  gtk_widget_get_allocation(GetNative(), &outer);
  GdkRectangle inner;
  GTK_FRAME_GET_CLASS(GetNative())->compute_child_allocation(
      GTK_FRAME(GetNative()), &inner);
  return SizeF(outer.width - inner.width, outer.height - inner.height);
}

}  // namespace nu
