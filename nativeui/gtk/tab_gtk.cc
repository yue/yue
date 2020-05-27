// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/tab.h"

#include <gtk/gtk.h>

#include "nativeui/gtk/util/widget_util.h"

namespace nu {

namespace {

void OnSwitchPage(GtkNotebook*, GtkWidget*, guint, Tab* tab) {
  tab->on_selected_page_change.Emit(tab);
}

}  // namespace

NativeView Tab::PlatformCreate() {
  GtkWidget* tab = gtk_notebook_new();
  g_signal_connect(tab, "switch-page", G_CALLBACK(OnSwitchPage), this);
  return tab;
}

void Tab::PlatformAddPage(const std::string& title, View* view) {
  gtk_notebook_append_page(GTK_NOTEBOOK(GetNative()),
                           view->GetNative(), gtk_label_new(title.c_str()));
}

void Tab::PlatformRemovePage(int index, View* view) {
  gtk_notebook_remove_page(GTK_NOTEBOOK(GetNative()), index);
}

void Tab::SelectPageAt(int index) {
  gtk_notebook_set_current_page(GTK_NOTEBOOK(GetNative()), index);
}

int Tab::GetSelectedPageIndex() const {
  return gtk_notebook_get_current_page(GTK_NOTEBOOK(GetNative()));
}

SizeF Tab::GetMinimumSize() const {
  return GetPreferredSizeForWidget(GetNative());
}

}  // namespace nu
