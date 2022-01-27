// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/picker.h"

#include <gtk/gtk.h>

#include "nativeui/gtk/util/widget_util.h"

namespace nu {

namespace {

void OnChanged(GObject* widget, Picker* picker) {
  if (g_object_get_data(widget, "ignore-change")) {
    g_object_set_data(widget, "ignore-change", nullptr);
    return;
  }
  picker->on_selection_change.Emit(picker);
}

void IterateComboBox(GtkComboBox* combobox,
                     const std::function<bool(char*)>& callback) {
  GtkTreeModel* model = gtk_combo_box_get_model(combobox);
  int column = gtk_combo_box_get_entry_text_column(combobox);
  GtkTreeIter iter;
  if (gtk_tree_model_get_iter_first(model, &iter)) {
    do {
      char* text = nullptr;
      gtk_tree_model_get(model, &iter, column, &text, -1);
      if (!callback(text))
        break;
    } while (gtk_tree_model_iter_next(model, &iter));
  }
}

}  // namespace

Picker::Picker() : Picker(gtk_combo_box_text_new()) {
}

Picker::Picker(NativeView view) {
  g_signal_connect(view, "changed", G_CALLBACK(OnChanged), this);
  TakeOverView(view);
  UpdateDefaultStyle();
}

void Picker::AddItem(const std::string& text) {
  // Check for duplicate item.
  bool duplicate = false;
  IterateComboBox(GTK_COMBO_BOX(GetNative()), [&duplicate, &text](char* item) {
    if (text == item) {
      duplicate = true;
      return false;
    }
    return true;
  });
  if (duplicate)
    return;

  // Add item and ignore event.
  g_object_set_data(G_OBJECT(GetNative()), "ignore-change", this);
  gtk_combo_box_text_append(
      GTK_COMBO_BOX_TEXT(GetNative()), nullptr, text.c_str());

  // Select the first item by default.
  if (GetSelectedItemIndex() == -1)
    SelectItemAt(0);
}

void Picker::RemoveItemAt(int index) {
  gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(GetNative()), index);
}

std::vector<std::string> Picker::GetItems() const {
  std::vector<std::string> result;
  IterateComboBox(GTK_COMBO_BOX(GetNative()), [&result](char* item) {
    result.emplace_back(item);
    return true;
  });
  return result;
}

void Picker::SelectItemAt(int index) {
  gtk_combo_box_set_active(GTK_COMBO_BOX(GetNative()), index);
}

std::string Picker::GetSelectedItem() const {
  return GetSelectedItemIndex() == -1 ?
     std::string() :
     gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(GetNative()));
}

int Picker::GetSelectedItemIndex() const {
  return gtk_combo_box_get_active(GTK_COMBO_BOX(GetNative()));
}

SizeF Picker::GetMinimumSize() const {
  return GetPreferredSizeForWidget(GetNative());
}

}  // namespace nu
