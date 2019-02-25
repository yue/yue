// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/undoable_text_buffer.h"

#include <gtk/gtk.h>

#include <stack>
#include <string>
#include <utility>

#include "nativeui/gtk/widget_util.h"

namespace nu {

namespace {

// Insert or Delete.
enum ActionType {
  INSERT,
  DELETE,
};

// An undoable action.
struct UndoableAction {
  // Insert action.
  UndoableAction(GtkTextIter* iter, std::string&& text)
      : type(INSERT), start(gtk_text_iter_get_offset(iter)), text(text) {}

  // Delete action.
  UndoableAction(GtkTextBuffer* buffer,
                 GtkTextIter* start_iter,
                 GtkTextIter* end_iter)
      : type(DELETE),
        start(gtk_text_iter_get_offset(start_iter)),
        end(gtk_text_iter_get_offset(end_iter)),
        text(gtk_text_buffer_get_text(buffer, start_iter, end_iter, TRUE)) {
    // Whether it is Delete or Backspace key.
    GtkTextIter insert_iter;
    gtk_text_buffer_get_iter_at_mark(buffer, &insert_iter,
                                     gtk_text_buffer_get_insert(buffer));
    is_delete = gtk_text_iter_get_offset(&insert_iter) <= start;
  }

  ActionType type;
  int start;
  int end;  // only used by delete action
  std::string text;
  bool is_delete;
};

// A structure holding the undo and redo stacks.
struct UndoableData {
  std::stack<UndoableAction> undo_stack;
  std::stack<UndoableAction> redo_stack;
  bool ignore_events = false;
};

void OnInsertText(GtkTextBuffer* buffer,
                  GtkTextIter* iter,
                  gchar* text, gint length,
                  UndoableData* data) {
  if (data->ignore_events)
    return;
  data->redo_stack = std::stack<UndoableAction>();
  data->undo_stack.emplace(iter, std::string(text, length));
}

void OnDeleteRange(GtkTextBuffer* buffer,
                   GtkTextIter* start_iter,
                   GtkTextIter* end_iter,
                   UndoableData* data) {
  if (data->ignore_events)
    return;
  data->redo_stack = std::stack<UndoableAction>();
  data->undo_stack.emplace(buffer, start_iter, end_iter);
}

}  // namespace

void TextBufferMakeUndoable(GtkTextBuffer* buffer) {
  UndoableData* data = new UndoableData;
  g_object_set_data_full(G_OBJECT(buffer), "undoable-data", data,
                         Delete<UndoableData>);
  g_signal_connect(buffer, "insert-text", G_CALLBACK(OnInsertText), data);
  g_signal_connect(buffer, "delete-range", G_CALLBACK(OnDeleteRange), data);
}

bool TextBufferIsUndoable(GtkTextBuffer* buffer) {
  return g_object_get_data(G_OBJECT(buffer), "undoable-data");
}

void TextBufferUndo(GtkTextBuffer* buffer) {
  auto* data = static_cast<UndoableData*>(
      g_object_get_data(G_OBJECT(buffer), "undoable-data"));
  if (data->undo_stack.empty())
    return;

  UndoableAction undo_action = std::move(data->undo_stack.top());
  data->undo_stack.pop();

  data->ignore_events = true;
  if (undo_action.type == INSERT) {
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_iter_at_offset(buffer, &start_iter, undo_action.start);
    gtk_text_buffer_get_iter_at_offset(
        buffer, &end_iter, undo_action.start + undo_action.text.length());
    gtk_text_buffer_delete(buffer, &start_iter, &end_iter);
    gtk_text_buffer_place_cursor(buffer, &start_iter);
  } else {
    GtkTextIter start_iter;
    gtk_text_buffer_get_iter_at_offset(buffer, &start_iter, undo_action.start);
    gtk_text_buffer_insert(buffer, &start_iter,
                           undo_action.text.data(), undo_action.text.length());
    if (undo_action.is_delete) {
      gtk_text_buffer_place_cursor(buffer, &start_iter);
    } else {
      GtkTextIter end_iter;
      gtk_text_buffer_get_iter_at_offset(buffer, &end_iter, undo_action.end);
      gtk_text_buffer_place_cursor(buffer, &end_iter);
    }
  }
  data->ignore_events = false;

  data->redo_stack.push(std::move(undo_action));
}

bool TextBufferCanUndo(GtkTextBuffer* buffer) {
  auto* data = static_cast<UndoableData*>(
      g_object_get_data(G_OBJECT(buffer), "undoable-data"));
  return !data->undo_stack.empty();
}

void TextBufferRedo(GtkTextBuffer* buffer) {
  auto* data = static_cast<UndoableData*>(
      g_object_get_data(G_OBJECT(buffer), "undoable-data"));
  if (data->redo_stack.empty())
    return;

  UndoableAction redo_action = std::move(data->redo_stack.top());
  data->redo_stack.pop();

  data->ignore_events = true;
  if (redo_action.type == INSERT) {
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_iter_at_offset(buffer, &start_iter, redo_action.start);
    gtk_text_buffer_insert(buffer, &start_iter,
                           redo_action.text.data(), redo_action.text.length());
    gtk_text_buffer_get_iter_at_offset(
        buffer, &end_iter, redo_action.start + redo_action.text.length());
    gtk_text_buffer_place_cursor(buffer, &end_iter);
  } else {
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_iter_at_offset(buffer, &start_iter, redo_action.start);
    gtk_text_buffer_get_iter_at_offset(buffer, &end_iter, redo_action.end);
    gtk_text_buffer_delete(buffer, &start_iter, &end_iter);
    gtk_text_buffer_place_cursor(buffer, &start_iter);
  }
  data->ignore_events = false;

  data->undo_stack.emplace(std::move(redo_action));
}

bool TextBufferCanRedo(GtkTextBuffer* buffer) {
  auto* data = static_cast<UndoableData*>(
      g_object_get_data(G_OBJECT(buffer), "undoable-data"));
  return !data->redo_stack.empty();
}

}  // namespace nu
