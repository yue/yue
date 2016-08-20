// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

namespace nu {

namespace {

void OnSizeAllocate(GtkWidget* view, GtkAllocation* alloc,
                    Container* container) {
  // Ignore empty sizes on initialization.
  if (alloc->x == -1 && alloc->y == -1 &&
      alloc->width == 1 && alloc->height == 1)
    return;

  container->Layout();
}

}  // namespace

void Container::PlatformInit() {
  set_view(gtk_fixed_new());
  g_object_ref_sink(view());
  gtk_widget_show(view());

  g_signal_connect_after(
      view(), "size-allocate", G_CALLBACK(OnSizeAllocate), this);
}

void Container::PlatformAddChildView(View* child) {
  gtk_container_add(GTK_CONTAINER(view()), child->view());
}

void Container::PlatformRemoveChildView(View* child) {
  gtk_container_remove(GTK_CONTAINER(view()), child->view());
}

}  // namespace nu
