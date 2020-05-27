// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/entry.h"

#include <gtk/gtk.h>

#include "nativeui/gtk/util/widget_util.h"

namespace nu {

namespace {

void OnActivate(GtkEntry*, Entry* entry) {
  entry->on_activate.Emit(entry);
}

void OnEntryTextChange(GtkEditable*, Entry* entry) {
  entry->on_text_change.Emit(entry);
}

}  // namespace

Entry::Entry(Type type) {
  TakeOverView(gtk_entry_new());
  UpdateDefaultStyle();

  if (type == Type::Password) {
    gtk_entry_set_visibility(GTK_ENTRY(GetNative()), false);
    gtk_entry_set_input_purpose(GTK_ENTRY(GetNative()),
                                GTK_INPUT_PURPOSE_PASSWORD);
  }

  g_signal_connect(GetNative(), "activate", G_CALLBACK(OnActivate), this);
  g_signal_connect(GetNative(), "changed", G_CALLBACK(OnEntryTextChange), this);
}

Entry::~Entry() {
}

void Entry::SetText(const std::string& text) {
  gtk_entry_set_text(GTK_ENTRY(GetNative()), text.c_str());
}

std::string Entry::GetText() const {
  return gtk_entry_get_text(GTK_ENTRY(GetNative()));
}

SizeF Entry::GetMinimumSize() const {
  return GetPreferredSizeForWidget(GetNative());
}

}  // namespace nu
