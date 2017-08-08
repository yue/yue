// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/text_edit.h"

#include <gtk/gtk.h>

#include "nativeui/gtk/widget_util.h"

namespace nu {

namespace {

void OnTextChange(GtkTextBuffer*, TextEdit* edit) {
  edit->on_text_change.Emit(edit);
}

}  // namespace

TextEdit::TextEdit() {
  GtkWidget* text_view = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_CHAR);
  gtk_widget_show(text_view);

  GtkWidget* scroll = gtk_scrolled_window_new(nullptr, nullptr);
  g_object_set_data(G_OBJECT(scroll), "text-view", text_view);
  gtk_container_add(GTK_CONTAINER(scroll), text_view);
  TakeOverView(scroll);

  GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
  g_signal_connect(buffer, "changed", G_CALLBACK(OnTextChange), this);
}

TextEdit::~TextEdit() {
}

std::string TextEdit::GetText() const {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  GtkTextIter start_iter, end_iter;
	gtk_text_buffer_get_start_iter(buffer, &start_iter);
	gtk_text_buffer_get_end_iter(buffer, &end_iter);
  return gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, false);
}

}  // namespace nu

