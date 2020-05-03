// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/combo_box.h"

#include <gtk/gtk.h>

namespace nu {

namespace {

void OnComTextChange(GObject* widget, ComboBox* combbox) {
  if (g_object_get_data(widget, "ignore-change")) {
    g_object_set_data(widget, "ignore-change", nullptr);
    return;
  }
  combbox->on_text_change.Emit(combbox);
}

}  // namespace

ComboBox::ComboBox() : Picker(gtk_combo_box_text_new_with_entry()) {
  GtkWidget* entry = gtk_bin_get_child(GTK_BIN(GetNative()));
  g_signal_connect(entry, "changed", G_CALLBACK(OnComTextChange), this);
}

ComboBox::~ComboBox() {
}

void ComboBox::SetText(const std::string& text) {
  GtkWidget* entry = gtk_bin_get_child(GTK_BIN(GetNative()));
  // Set text and ignore event.
  g_object_set_data(G_OBJECT(entry), "ignore-change", this);
  gtk_entry_set_text(GTK_ENTRY(entry), text.c_str());
}

std::string ComboBox::GetText() const {
  GtkWidget* entry = gtk_bin_get_child(GTK_BIN(GetNative()));
  return gtk_entry_get_text(GTK_ENTRY(entry));
}

void ComboBox::AddItem(const std::string& text) {
  // Add item and ignore event.
  g_object_set_data(G_OBJECT(GetNative()), "ignore-change", this);
  gtk_combo_box_text_append(
      GTK_COMBO_BOX_TEXT(GetNative()), nullptr, text.c_str());
}

}  // namespace nu
