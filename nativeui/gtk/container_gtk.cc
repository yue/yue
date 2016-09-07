// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#include "nativeui/gtk/container_container.h"

namespace nu {

void Container::PlatformInit() {
  TakeOverView(nu_container_new(this));
}

void Container::PlatformDestroy() {
  // The widget relies on Container to get children, so we must ensure the
  // widget is destroyed before this class.
  gtk_widget_destroy(view());
  TakeOverView(nullptr);
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
