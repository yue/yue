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
  TakeOverView(gtk_fixed_new());

  // Give the container a small initial size, otherwise GTK might give warnings.
  GdkRectangle rect = { 0, 0, 10, 10 };
  gtk_widget_size_allocate(view(), &rect);

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
