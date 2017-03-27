// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/button.h"

#include <gtk/gtk.h>

#include "nativeui/gtk/widget_util.h"

namespace nu {

namespace {

void OnClick(GtkButton*, Button* button) {
  button->on_click.Emit();
}

}  // namespace

Button::Button(const std::string& title, Type type) {
  if (type == Normal)
    TakeOverView(gtk_button_new_with_label(title.c_str()));
  else if (type == CheckBox)
    TakeOverView(gtk_check_button_new_with_label(title.c_str()));
  else if (type == Radio)
    TakeOverView(gtk_radio_button_new_with_label(nullptr, title.c_str()));

  SetDefaultStyle(SizeF(GetPreferredSizeForWidget(GetNative())));
  g_signal_connect(GetNative(), "clicked", G_CALLBACK(OnClick), this);
}

Button::~Button() {
}

void Button::SetTitle(const std::string& title) {
  gtk_button_set_label(GTK_BUTTON(GetNative()), title.c_str());
  SetDefaultStyle(SizeF(GetPreferredSizeForWidget(GetNative())));
}

std::string Button::GetTitle() const {
  return gtk_button_get_label(GTK_BUTTON(GetNative()));
}

void Button::SetChecked(bool checked) {
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GetNative()), checked);
}

bool Button::IsChecked() const {
  return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GetNative()));
}

}  // namespace nu
