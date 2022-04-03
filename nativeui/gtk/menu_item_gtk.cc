// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_item.h"

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "nativeui/gtk/util/undoable_text_buffer.h"
#include "nativeui/menu.h"
#include "nativeui/menu_bar.h"
#include "nativeui/window.h"

namespace nu {

namespace {

// Maps roles to stock IDs.
struct {
  const gchar* stock_id;
  const char* webkit_command;
} g_edit_map[] = {
  { GTK_STOCK_COPY, WEBKIT_EDITING_COMMAND_COPY },
  { GTK_STOCK_CUT, WEBKIT_EDITING_COMMAND_CUT },
  { GTK_STOCK_PASTE, WEBKIT_EDITING_COMMAND_PASTE },
  { GTK_STOCK_SELECT_ALL, WEBKIT_EDITING_COMMAND_SELECT_ALL },
  { GTK_STOCK_UNDO, WEBKIT_EDITING_COMMAND_UNDO },
  { GTK_STOCK_REDO, WEBKIT_EDITING_COMMAND_REDO },
};

static_assert(
    std::size(g_edit_map) == static_cast<size_t>(MenuItem::Role::Redo) + 1,
    "Stock edit items should map the roles");

// Handling role item clicking.
void OnRoleClick(GtkWidget*, MenuItem* item) {
  // Get the window.
  MenuBase* menu = item->FindTopLevelMenu();
  if (!menu || menu->GetClassName() != MenuBar::kClassName)
    return;
  Window* window = static_cast<MenuBar*>(menu)->GetWindow();
  if (!window)
    return;
  // Window roles.
  switch (item->GetRole()) {
    case MenuItem::Role::Minimize:
      window->Minimize();
      return;
    case MenuItem::Role::Maximize:
      window->Maximize();
      return;
    case MenuItem::Role::CloseWindow:
      window->Close();
      return;
    default:
      break;
  }
  // Get the focused widget.
  GtkWidget* widget = gtk_window_get_focus(window->GetNative());
  if (!widget)
    return;
  // Edit roles.
  if (WEBKIT_IS_WEB_VIEW(widget)) {
    webkit_web_view_execute_editing_command(
        WEBKIT_WEB_VIEW(widget),
        g_edit_map[static_cast<int>(item->GetRole())].webkit_command);
  } else {
    switch (item->GetRole()) {
      case MenuItem::Role::Copy:
        if (GTK_IS_ENTRY(widget) || GTK_IS_TEXT_VIEW(widget))
          g_signal_emit_by_name(widget, "copy-clipboard", nullptr);
        break;
      case MenuItem::Role::Cut:
        if (GTK_IS_ENTRY(widget) || GTK_IS_TEXT_VIEW(widget))
          g_signal_emit_by_name(widget, "cut-clipboard", nullptr);
        break;
      case MenuItem::Role::Paste:
        if (GTK_IS_ENTRY(widget) || GTK_IS_TEXT_VIEW(widget))
          g_signal_emit_by_name(widget, "paste-clipboard", nullptr);
        break;
      case MenuItem::Role::SelectAll:
        if (GTK_IS_TEXT_VIEW(widget))
          g_signal_emit_by_name(widget, "select-all", TRUE, nullptr);
        else if (GTK_IS_ENTRY(widget))
          gtk_widget_grab_focus(widget);
        break;
      case MenuItem::Role::Undo:
        if (GTK_IS_TEXT_VIEW(widget)) {
          auto* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
          if (TextBufferIsUndoable(buffer))
            TextBufferUndo(buffer);
        }
        break;
      case MenuItem::Role::Redo:
        if (GTK_IS_TEXT_VIEW(widget)) {
          auto* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
          if (TextBufferIsUndoable(buffer))
            TextBufferRedo(buffer);
        }
        break;
      default:
        break;
    }
  }
}

// Normal handling of the clicking.
void OnItemClick(GtkMenuItem*, MenuItem* item) {
  item->EmitClick();
}

}  // namespace

void MenuItem::Click() {
  gtk_menu_item_activate(menu_item_);
}

void MenuItem::SetLabel(const std::string& label) {
  gtk_menu_item_set_label(menu_item_, label.c_str());
}

std::string MenuItem::GetLabel() const {
  return gtk_menu_item_get_label(menu_item_);
}

void MenuItem::SetChecked(bool checked) {
  if (GTK_IS_CHECK_MENU_ITEM(menu_item_))
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item_), checked);
}

bool MenuItem::IsChecked() const {
  if (GTK_IS_CHECK_MENU_ITEM(menu_item_))
    return gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu_item_));
  return false;
}

void MenuItem::SetEnabled(bool enabled) {
  gtk_widget_set_sensitive(GTK_WIDGET(menu_item_), enabled);
}

bool MenuItem::IsEnabled() const {
  return gtk_widget_get_sensitive(GTK_WIDGET(menu_item_));
}

void MenuItem::SetVisible(bool visible) {
  gtk_widget_set_visible(GTK_WIDGET(menu_item_), visible);
}

bool MenuItem::IsVisible() const {
  return gtk_widget_get_visible(GTK_WIDGET(menu_item_));
}

void MenuItem::PlatformInit() {
  GtkWidget* item;
  GtkStockItem stock;
  if (role_ <= Role::Redo &&
      gtk_stock_lookup(g_edit_map[static_cast<int>(role_)].stock_id, &stock)) {
    // Create item from the stock.
    item = gtk_image_menu_item_new_from_stock(stock.stock_id, nullptr);
    SetAccelerator(Accelerator(static_cast<KeyboardCode>(stock.keyval),
                               stock.modifier));
  } else {
    // Otherwise create custom item.
    switch (type_) {
      case Type::Label: case Type::Submenu: item = gtk_menu_item_new(); break;
      case Type::Radio: item = gtk_radio_menu_item_new(nullptr); break;
      case Type::Checkbox: item = gtk_check_menu_item_new(); break;
      case Type::Separator: item = gtk_separator_menu_item_new(); break;
    }
  }

  if (role_ < Role::ItemCount)
    g_signal_connect(item, "activate", G_CALLBACK(OnRoleClick), this);
  else
    g_signal_connect(item, "activate", G_CALLBACK(OnItemClick), this);

  gtk_widget_show(item);
  g_object_ref_sink(item);
  menu_item_ = GTK_MENU_ITEM(item);
}

void MenuItem::PlatformDestroy() {
  gtk_widget_destroy(GTK_WIDGET(menu_item_));
  g_object_unref(menu_item_);
}

void MenuItem::PlatformSetSubmenu(Menu* submenu) {
  gtk_menu_item_set_submenu(menu_item_, GTK_WIDGET(submenu->GetNative()));
}

}  // namespace nu
