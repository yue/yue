// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/entry.h"

#include "nativeui/gtk/widget_util.h"

namespace nu {

namespace {

void OnActivate(GtkEntry*, Entry* entry) {
  entry->on_activate.Emit();
}

void OnTextChange(GtkEditable*, Entry* entry) {
  entry->on_text_change.Emit();
}

}  // namespace

Entry::Entry() {
  TakeOverView(gtk_entry_new());
  SetDefaultStyle(SizeF(GetPreferredSizeForWidget(view())));

  g_signal_connect(view(), "activate", G_CALLBACK(OnActivate), this);
  g_signal_connect(view(), "changed", G_CALLBACK(OnTextChange), this);
}

Entry::~Entry() {
}

void Entry::SetText(const std::string& text) {
  gtk_entry_set_text(GTK_ENTRY(view()), text.c_str());
}

std::string Entry::GetText() const {
  return gtk_entry_get_text(GTK_ENTRY(view()));
}

}  // namespace nu
