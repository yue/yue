// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/undoable_text_buffer.h"

#include <gtk/gtk.h>

#include <stack>
#include <string>
#include <utility>

#include "base/strings/string_util.h"
#include "nativeui/gtk/widget_util.h"

namespace nu {

namespace {

inline bool IsWhiteSpace(const std::string& text) {
  return text.length() == 1 && base::IsAsciiWhitespace(text[0]);
}

// Insert or Delete.
enum ActionType {
  INSERT,
  DELETE,
};

// An undoable action.
struct UndoableAction {
  // Insert action.
  UndoableAction(GtkTextIter* iter, const std::string& text)
      : type(INSERT),
        start(gtk_text_iter_get_offset(iter)),
        text(text),
        mergeable(!text.empty()) {
  }

  // Delete action.
  UndoableAction(GtkTextBuffer* buffer,
                 GtkTextIter* start_iter,
                 GtkTextIter* end_iter)
      : type(INSERT),
        start(gtk_text_iter_get_offset(start_iter)),
        end(gtk_text_iter_get_offset(end_iter)),
        text(gtk_text_buffer_get_text(buffer, start_iter, end_iter, TRUE)),
        mergeable(IsWhiteSpace(text)) {
    // Whether it is Delete or Backspace key.
    GtkTextIter insert_iter;
    gtk_text_buffer_get_iter_at_mark(buffer, &insert_iter,
                                     gtk_text_buffer_get_insert(buffer));
    delete_key_used = gtk_text_iter_get_offset(&insert_iter) <= start;
  }

  ActionType type;
  int start;
  int end;
  std::string text;
  bool mergeable;
  bool delete_key_used;
};

// A structure holding the undo and redo stacks.
struct UndoableData {
  std::stack<UndoableAction> undo_stack;
  std::stack<UndoableAction> redo_stack;
  bool not_undoable_action = false;
  bool undo_in_progress = true;
};

void OnInsertText(GtkTextBuffer* buffer,
                  GtkTextIter* iter,
                  gchar* text, gint length,
                  UndoableData* data) {
  if (!data->undo_in_progress)
    data->redo_stack = std::stack<UndoableAction>();
  if (data->not_undoable_action)
    return;
  UndoableAction cur(iter, std::string(text, length));
  // Check whether we can merge multiple inserts.
  // Will try to merge words or whitespace;
  // Can't merge if |prev| and |cur| are not mergeable in the first place;
  // Can't merge when user set the input bar somewhere else;
  // Can't merge across word boundaries.
  bool mergeable = false;
  if (!data->undo_stack.empty()) {
    UndoableAction& prev = data->undo_stack.top();
    mergeable =
        prev.mergeable && cur.mergeable &&
        (prev.type == cur.type) &&
        (static_cast<size_t>(cur.start) == prev.start + prev.text.length()) &&
        (IsWhiteSpace(prev.text) != IsWhiteSpace(cur.text));
  }
  if (mergeable)
    data->undo_stack.top().text += cur.text;
  else
    data->undo_stack.push(std::move(cur));
}

void OnDeleteRange(GtkTextBuffer* buffer,
                   GtkTextIter* start_iter,
                   GtkTextIter* end_iter,
                   UndoableData* data) {
  if (!data->undo_in_progress)
    data->redo_stack = std::stack<UndoableAction>();
  if (data->not_undoable_action)
    return;
  UndoableAction cur(buffer, start_iter, end_iter);
  // Check whether we can merge multiple deletions.
  // Will try to merge words or whitespace;
  // Can't merge if |prev| and |cur| are not mergeable in the first place;
  // can't merge if delete and backspace key were both used;
  // Can't merge across word boundaries.
  bool mergeable = false;
  if (!data->undo_stack.empty()) {
    UndoableAction& prev = data->undo_stack.top();
    mergeable =
        prev.mergeable && cur.mergeable &&
        (prev.type == cur.type) &&
        (prev.delete_key_used == cur.delete_key_used) &&
        (prev.start == cur.start || prev.start == cur.end) &&
        (IsWhiteSpace(prev.text) != IsWhiteSpace(cur.text));
  }
  if (mergeable) {
    UndoableAction& prev = data->undo_stack.top();
    if (prev.start == cur.start) {  // delete key used
      prev.text += cur.text;
      prev.end += (cur.end - cur.start);
    } else {  // backspace key used
      prev.text = cur.text + prev.text;
      prev.start = cur.start;
    }
    return;
  }
  data->undo_stack.push(std::move(cur));
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
  data->not_undoable_action = true;
  data->undo_in_progress = true;
  UndoableAction undo_action = data->undo_stack.top();
  data->undo_stack.pop();
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
    if (undo_action.delete_key_used) {
      gtk_text_buffer_place_cursor(buffer, &start_iter);
    } else {
      GtkTextIter end_iter;
      gtk_text_buffer_get_iter_at_offset(buffer, &end_iter, undo_action.end);
      gtk_text_buffer_place_cursor(buffer, &end_iter);
    }
  }
  data->redo_stack.push(std::move(undo_action));
  data->not_undoable_action = false;
  data->undo_in_progress = false;
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
  data->not_undoable_action = true;
  data->undo_in_progress = true;
  UndoableAction redo_action = data->redo_stack.top();
  data->redo_stack.pop();
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
  data->undo_stack.push(std::move(redo_action));
  data->not_undoable_action = false;
  data->undo_in_progress = false;
}

bool TextBufferCanRedo(GtkTextBuffer* buffer) {
  auto* data = static_cast<UndoableData*>(
      g_object_get_data(G_OBJECT(buffer), "undoable-data"));
  return !data->redo_stack.empty();
}

}  // namespace nu
