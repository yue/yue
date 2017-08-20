// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/file_dialog.h"

#include <gtk/gtk.h>

#include "nativeui/gtk/widget_util.h"
#include "nativeui/window.h"

namespace nu {

FileDialog::FileDialog(NativeFileDialog dialog) : dialog_(dialog) {
}

FileDialog::~FileDialog() {
  gtk_widget_destroy(GTK_WIDGET(dialog_));
}

base::FilePath FileDialog::GetResult() const {
  char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog_));
  base::FilePath path = AddExtensionForFilename(dialog_, filename);
  g_free(filename);
  return path;
}

bool FileDialog::Run() {
  gtk_window_set_modal(GTK_WINDOW(dialog_), true);
  gtk_widget_show_all(GTK_WIDGET(dialog_));
  bool r = gtk_dialog_run(GTK_DIALOG(dialog_)) == GTK_RESPONSE_ACCEPT;
  gtk_widget_hide(GTK_WIDGET(dialog_));
  return r;
}

bool FileDialog::RunForWindow(Window* window) {
  gtk_window_set_transient_for(GTK_WINDOW(dialog_), window->GetNative());
  return Run();
}

void FileDialog::SetTitle(const std::string& title) {
  gtk_window_set_title(GTK_WINDOW(dialog_), title.c_str());
}

void FileDialog::SetButtonLabel(const std::string& label) {
  GtkWidget* button = gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog_),
                                                         GTK_RESPONSE_ACCEPT);
  gtk_button_set_label(GTK_BUTTON(button), label.c_str());
}

void FileDialog::SetFilename(const std::string& filename) {
  gtk_file_chooser_set_current_name(dialog_, filename.c_str());
}

void FileDialog::SetFolder(const base::FilePath& folder) {
  gtk_file_chooser_set_current_folder(dialog_, folder.value().c_str());
}

}  // namespace nu
