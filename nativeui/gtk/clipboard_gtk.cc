// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/clipboard.h"

#include "base/notreached.h"
#include "nativeui/gtk/util/clipboard_util.h"

namespace nu {

namespace {

void ClipboardGet(GtkClipboard* clipboard,
                  GtkSelectionData* selection,
                  guint info,
                  std::vector<Clipboard::Data>* objects) {
  DCHECK_LT(info, objects->size());
  FillSelection(selection, (*objects)[info]);
}

void ClipboardClear(GtkClipboard* clipboard,
                    std::vector<Clipboard::Data>* objects) {
  delete objects;
}

void OnClipboardOnwerChange(GtkClipboard*, GdkEvent*, Clipboard* clipboard) {
  clipboard->on_change.Emit(clipboard);
}

}  // namespace

NativeClipboard Clipboard::PlatformCreate(Type type) {
  switch (type) {
    case Type::CopyPaste:
      return gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    case Type::Selection:
      return gtk_clipboard_get(GDK_SELECTION_PRIMARY);
    default:
      NOTREACHED() << "Type::Count is not a valid clipboard type";
  }
  return gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
}

void Clipboard::PlatformDestroy() {
}

bool Clipboard::IsDataAvailable(Data::Type type) const {
  return IsDataAvailableInClipboard(clipboard_, type);
}

Clipboard::Data Clipboard::GetData(Data::Type type) const {
  return GetDataFromClipboard(clipboard_, type);
}

void Clipboard::SetData(std::vector<Data> objects) {
  GtkTargetList* targets = gtk_target_list_new(0, 0);
  for (size_t i = 0; i < objects.size(); ++i)
    FillTargetList(targets, objects[i].type(), i);

  int number = 0;
  GtkTargetEntry* table = gtk_target_table_new_from_list(targets, &number);
  if (table && number > 0) {
    gtk_clipboard_set_with_data(clipboard_, table, number,
                                (GtkClipboardGetFunc)ClipboardGet,
                                (GtkClipboardClearFunc)ClipboardClear,
                                new std::vector<Data>(std::move(objects)));
    gtk_clipboard_set_can_store(clipboard_, nullptr, 0);
  } else {
    gtk_clipboard_clear(clipboard_);
  }

  if (table)
    gtk_target_table_free(table, number);
  gtk_target_list_unref(targets);
}

void Clipboard::PlatformStartWatching() {
  DCHECK_EQ(signal_, 0u);
  signal_ = g_signal_connect(GetNative(), "owner-change",
                             G_CALLBACK(OnClipboardOnwerChange), this);
}

void Clipboard::PlatformStopWatching() {
  DCHECK_GT(signal_, 0u);
  g_signal_handler_disconnect(GetNative(), signal_);
  signal_ = 0;
}

}  // namespace nu
