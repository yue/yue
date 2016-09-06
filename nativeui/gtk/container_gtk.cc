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
  g_signal_connect_after(
      view(), "size-allocate", G_CALLBACK(OnSizeAllocate), this);
}

void Container::PlatformAddChildView(View* child) {
  // If we are adding a radio button, we check whether there is already a radio
  // button in the container, and join radio group if so.
  if (GTK_IS_RADIO_BUTTON(child->view())) {
    for (int i = 0; i < child_count(); ++i) {
      GtkWidget* widget = child_at(i)->view();
      if (GTK_IS_RADIO_BUTTON(widget)) {
        gtk_radio_button_join_group(GTK_RADIO_BUTTON(child->view()),
                                    GTK_RADIO_BUTTON(widget));
        break;
      }
    }
  }

  gtk_container_add(GTK_CONTAINER(view()), child->view());
}

void Container::PlatformRemoveChildView(View* child) {
  gtk_container_remove(GTK_CONTAINER(view()), child->view());
}

}  // namespace nu
