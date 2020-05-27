// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/text_edit.h"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "nativeui/gfx/font.h"
#include "nativeui/gtk/util/undoable_text_buffer.h"
#include "nativeui/gtk/util/widget_util.h"

namespace nu {

namespace {

GtkPolicyType ToGTK(Scroll::Policy policy) {
  if (policy == Scroll::Policy::Always)
    return GTK_POLICY_ALWAYS;
  else if (policy == Scroll::Policy::Never)
    return GTK_POLICY_NEVER;
  else
    return GTK_POLICY_AUTOMATIC;
}

void OnTextChange(GtkTextBuffer*, TextEdit* edit) {
  edit->on_text_change.Emit(edit);
}

gboolean OnKeyPress(GtkWidget*, GdkEventKey* event, TextEdit* edit) {
  if (event->type == GDK_KEY_PRESS && event->keyval == GDK_KEY_Return &&
      edit->should_insert_new_line)
    return !edit->should_insert_new_line(edit);
  return FALSE;
}

}  // namespace

TextEdit::TextEdit() {
  GtkWidget* text_view = gtk_text_view_new();
  g_signal_connect(text_view, "key-press-event", G_CALLBACK(OnKeyPress), this);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_CHAR);
  gtk_widget_show(text_view);

  GtkWidget* scroll = gtk_scrolled_window_new(nullptr, nullptr);
  g_object_set_data(G_OBJECT(scroll), "text-view", text_view);
  gtk_container_add(GTK_CONTAINER(scroll), text_view);
  TakeOverView(scroll);

  GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
  TextBufferMakeUndoable(buffer);
  g_signal_connect(buffer, "changed", G_CALLBACK(OnTextChange), this);
}

TextEdit::~TextEdit() {
}

void TextEdit::SetText(const std::string& text) {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  gtk_text_buffer_set_text(buffer, text.c_str(), text.size());
}

std::string TextEdit::GetText() const {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  GtkTextIter start_iter, end_iter;
  gtk_text_buffer_get_start_iter(buffer, &start_iter);
  gtk_text_buffer_get_end_iter(buffer, &end_iter);
  char* text = gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, false);
  std::string result(text);
  g_free(text);
  return result;
}

void TextEdit::Redo() {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  TextBufferRedo(buffer);
}

bool TextEdit::CanRedo() const {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  return TextBufferCanRedo(buffer);
}

void TextEdit::Undo() {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  TextBufferUndo(buffer);
}

bool TextEdit::CanUndo() const {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  return TextBufferCanUndo(buffer);
}

void TextEdit::Cut() {
  GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  gtk_text_buffer_cut_clipboard(buffer, clipboard, TRUE);
}

void TextEdit::Copy() {
  GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  gtk_text_buffer_copy_clipboard(buffer, clipboard);
}

void TextEdit::Paste() {
  GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  gtk_text_buffer_paste_clipboard(buffer, clipboard, nullptr, TRUE);
}

void TextEdit::SelectAll() {
  GtkTextView* text_view =
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view"));
  g_signal_emit_by_name(text_view, "select-all", TRUE, nullptr);
}

std::tuple<int, int> TextEdit::GetSelectionRange() const {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  GtkTextIter start_iter, end_iter;
  gint pos;
  if (gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter)) {
    return std::make_tuple(gtk_text_iter_get_offset(&start_iter),
                           gtk_text_iter_get_offset(&end_iter));
  } else {
    g_object_get(buffer, "cursor-position", &pos, NULL);
    return std::make_tuple(pos, pos);
  }
}

void TextEdit::SelectRange(int start, int end) {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  GtkTextIter start_iter, end_iter;
  gtk_text_buffer_get_iter_at_offset(buffer, &start_iter, start);
  gtk_text_buffer_get_iter_at_offset(buffer, &end_iter, end);
  gtk_text_buffer_select_range(buffer, &start_iter, &end_iter);
}

std::string TextEdit::GetTextInRange(int start, int end) const {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  GtkTextIter start_iter, end_iter;
  gtk_text_buffer_get_iter_at_offset(buffer, &start_iter, start);
  gtk_text_buffer_get_iter_at_offset(buffer, &end_iter, end);
  char* text = gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, false);
  std::string result(text);
  g_free(text);
  return result;
}

void TextEdit::InsertText(const std::string& text) {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  gtk_text_buffer_insert_at_cursor(buffer, text.c_str(), text.size());
}

void TextEdit::InsertTextAt(const std::string& text, int pos) {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  GtkTextIter iter;
  gtk_text_buffer_get_iter_at_offset(buffer, &iter, pos);
  gtk_text_buffer_insert(buffer, &iter, text.c_str(), text.size());
}

void TextEdit::Delete() {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  gtk_text_buffer_delete_selection(buffer, FALSE, TRUE);
}

void TextEdit::DeleteRange(int start, int end) {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  GtkTextIter start_iter, end_iter;
  gtk_text_buffer_get_iter_at_offset(buffer, &start_iter, start);
  gtk_text_buffer_get_iter_at_offset(buffer, &end_iter, end);
  gtk_text_buffer_delete(buffer, &start_iter, &end_iter);
}

void TextEdit::SetOverlayScrollbar(bool overlay) {
  if (GtkVersionCheck(3, 16))
    gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(GetNative()),
                                              overlay);
}

void TextEdit::SetScrollbarPolicy(Scroll::Policy h_policy,
                                  Scroll::Policy v_policy) {
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(GetNative()),
                                 ToGTK(h_policy), ToGTK(v_policy));
}

RectF TextEdit::GetTextBounds() const {
  // There is no reliable way to get the real text extends with GtkTextView
  // APIs, getting widget preferred size or scroll window upper value does not
  // work when widget is not mapped, and it returns wrong values with empty
  // lines.
  std::string text = GetText();
  PangoLayout* layout =
      pango_layout_new(gtk_widget_get_pango_context(GetNative()));
  if (font())
    pango_layout_set_font_description(layout, font()->GetNative());
  pango_layout_set_text(layout, text.c_str(), text.size());
  int w, h;
  pango_layout_get_pixel_size(layout, &w, &h);
  g_object_unref(layout);
  return RectF(0, 0, w, h);
}

}  // namespace nu
