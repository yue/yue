// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/button.h"

namespace nu {

namespace {

void OnClick(GtkButton* width, Button* button) {
  button->on_click.Notify();
}

}  // namespace

Button::Button(const std::string& title) {
  set_view(gtk_button_new_with_label(title.c_str()));
  g_object_ref_sink(view());
  gtk_widget_show(view());

  g_signal_connect(view(), "clicked", G_CALLBACK(OnClick), this);
}

Button::~Button() {
}

void Button::SetTitle(const std::string& title) {
  gtk_button_set_label(GTK_BUTTON(view()), title.c_str());
}

std::string Button::GetTitle() const {
  return gtk_button_get_label(GTK_BUTTON(view()));
}

}  // namespace nu
