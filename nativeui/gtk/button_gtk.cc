// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/button.h"

#include <gtk/gtk.h>

#include "nativeui/gfx/image.h"
#include "nativeui/gtk/nu_image.h"
#include "nativeui/gtk/widget_util.h"

namespace nu {

namespace {

void OnClick(GtkButton*, Button* button) {
  button->on_click.Emit(button);
}

}  // namespace

Button::Button(const std::string& title, Type type) {
  if (type == Type::Normal)
    TakeOverView(gtk_button_new_with_label(title.c_str()));
  else if (type == Type::Checkbox)
    TakeOverView(gtk_check_button_new_with_label(title.c_str()));
  else if (type == Type::Radio)
    TakeOverView(gtk_radio_button_new_with_label(nullptr, title.c_str()));

  SetDefaultStyle(GetPreferredSizeForWidget(GetNative()));
  g_signal_connect(GetNative(), "clicked", G_CALLBACK(OnClick), this);
}

Button::~Button() {
}

void Button::SetTitle(const std::string& title) {
  gtk_button_set_label(GTK_BUTTON(GetNative()), title.c_str());
  SetDefaultStyle(GetPreferredSizeForWidget(GetNative()));
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

void Button::SetImage(Image* image) {
  GtkButton* button = GTK_BUTTON(GetNative());
  // Explicity nullptr for empty title otherwise space would be reserved for
  // title and image would not show in the middle.
  if (GetTitle().empty())
    gtk_button_set_label(button, nullptr);
  // Show image and title together.
  gtk_button_set_always_show_image(button, true);
  // Use custom NUImage widget instead of GtkImage because the latter does not
  // have high DPI support.
  gtk_button_set_image(button, nu_image_new(image));
  // Update style.
  SetDefaultStyle(GetPreferredSizeForWidget(GetNative()));
}

}  // namespace nu
