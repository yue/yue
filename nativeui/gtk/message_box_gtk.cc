// Copyright 2020 Cheng Zhao. All rights reserved.
// Copyright 2013 GitHub, Inc.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/message_box.h"

#include "nativeui/gfx/image.h"
#include "nativeui/gtk/nu_image.h"
#include "nativeui/window.h"

namespace nu {

namespace {

inline bool IsCancelResponse(int res) {
  return res == GTK_RESPONSE_NONE || res == GTK_RESPONSE_DELETE_EVENT;
}

void OnMessageBoxResponse(GtkWidget* widget, int res, MessageBox* self) {
  gtk_widget_hide(widget);
  if (IsCancelResponse(res))
    self->OnClose();
  else
    self->OnClose(res);
}

}  // namespace

MessageBox::MessageBox() {
  box_ = GTK_MESSAGE_DIALOG(
      gtk_message_dialog_new(nullptr,                         // parent
                             static_cast<GtkDialogFlags>(0),  // flags
                             GTK_MESSAGE_OTHER,               // type
                             GTK_BUTTONS_NONE,                // buttons
                             nullptr));
  gtk_window_set_modal(GTK_WINDOW(box_), TRUE);
  g_signal_connect(box_, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete),
                   nullptr);
  g_signal_connect(box_, "response", G_CALLBACK(OnMessageBoxResponse), this);
}

MessageBox::~MessageBox() {
  gtk_widget_destroy(GTK_WIDGET(box_));
}

int MessageBox::PlatformRun() {
  return PlatformRunForWindow(nullptr);
}

int MessageBox::PlatformRunForWindow(Window* window) {
  gtk_window_set_transient_for(GTK_WINDOW(box_),
                               window ? window->GetNative() : nullptr);
  int res = gtk_dialog_run(GTK_DIALOG(box_));
  return IsCancelResponse(res) ? cancel_response_ : res;
}

void MessageBox::PlatformShow() {
  PlatformShowForWindow(nullptr);
}

void MessageBox::PlatformShowForWindow(Window* window) {
  gtk_window_set_transient_for(GTK_WINDOW(box_),
                               window ? window->GetNative() : nullptr);
  gtk_widget_show(GTK_WIDGET(box_));
  gtk_window_present(GTK_WINDOW(box_));
}

void MessageBox::PlatformClose() {
  gtk_window_close(GTK_WINDOW(box_));
}

void MessageBox::SetType(Type type) {
  GValue val = G_VALUE_INIT;
  g_value_init(&val, GTK_TYPE_MESSAGE_TYPE);
  switch (type) {
    case MessageBox::Type::None:
      g_value_set_enum(&val, GTK_MESSAGE_OTHER);
      break;
    case MessageBox::Type::Information:
      g_value_set_enum(&val, GTK_MESSAGE_INFO);
      break;
    case MessageBox::Type::Warning:
      g_value_set_enum(&val, GTK_MESSAGE_WARNING);
      break;
    case MessageBox::Type::Error:
      g_value_set_enum(&val, GTK_MESSAGE_ERROR);
      break;
  }
  g_object_set_property(G_OBJECT(box_), "message-type", &val);
  g_value_unset(&val);
}

void MessageBox::SetTitle(const std::string& title) {
  gtk_window_set_title(GTK_WINDOW(box_), title.c_str());
}

void MessageBox::AddButton(const std::string& title, int response) {
  gtk_dialog_add_button(GTK_DIALOG(box_), title.c_str(), response);
  if (default_response_ && *default_response_ == response)
    gtk_dialog_set_default_response(GTK_DIALOG(box_), *default_response_);
}

void MessageBox::PlatformSetDefaultResponse() {
  gtk_dialog_set_default_response(GTK_DIALOG(box_), *default_response_);
}

void MessageBox::PlatformSetCancelResponse() {
}

void MessageBox::SetText(const std::string& text) {
  GValue val = G_VALUE_INIT;
  g_value_init(&val, G_TYPE_STRING);
  g_value_set_string(&val, text.c_str());
  g_object_set_property(G_OBJECT(box_), "text", &val);
  g_value_unset(&val);
}

void MessageBox::SetInformativeText(const std::string& text) {
  GValue val = G_VALUE_INIT;
  g_value_init(&val, G_TYPE_STRING);
  g_value_set_string(&val, text.c_str());
  g_object_set_property(G_OBJECT(box_), "secondary-text", &val);
  g_value_unset(&val);
}

void MessageBox::SetAccessoryView(scoped_refptr<View> view) {
  GtkContainer* message_area =
      GTK_CONTAINER(gtk_message_dialog_get_message_area(box_));
  if (accessory_view_) {
    gtk_container_remove(message_area, accessory_view_->GetNative());
    gtk_widget_set_size_request(accessory_view_->GetNative(), -1, -1);
  }
  Rect b = view->GetPixelBounds();
  gtk_widget_set_size_request(view->GetNative(), b.width(), b.height());
  gtk_container_add(GTK_CONTAINER(message_area), view->GetNative());
  accessory_view_ = std::move(view);
}

void MessageBox::PlatformSetImage(Image* image) {
  if (image) {
    GtkWidget* widget = nu_image_new(image);
    nu_image_set_size(NU_IMAGE(widget), Size(48, 48));
    gtk_widget_show(widget);
    gtk_message_dialog_set_image(box_, widget);
  } else {
    gtk_message_dialog_set_image(box_, nullptr);
  }
}

}  // namespace nu
