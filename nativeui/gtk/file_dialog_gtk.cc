// Copyright 2017 Cheng Zhao. All rights reserved.
// Copyright 2014 GitHub, Inc.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/file_dialog.h"

#include <gtk/gtk.h>

#include "base/strings/string_util.h"
#include "nativeui/window.h"

namespace nu {

namespace {

// Makes sure that .jpg also shows .JPG.
gboolean FileFilterCaseInsensitive(const GtkFileFilterInfo* file_info,
                                   const char* file_extension) {
  // Makes .* file extension matches all file types.
  if (strcmp(file_extension, ".*") == 0)
    return true;
  return base::EndsWith(
    file_info->filename,
    file_extension, base::CompareCase::INSENSITIVE_ASCII);
}

}  // namespace

FileDialog::FileDialog(NativeFileDialog dialog) : dialog_(dialog) {
}

FileDialog::~FileDialog() {
  gtk_widget_destroy(GTK_WIDGET(dialog_));
}

base::FilePath FileDialog::GetResult() const {
  char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog_));
  base::FilePath path = AddExtensionForFilename(filename);
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

void FileDialog::SetOptions(int options) {
  if (options & OPTION_PICK_FOLDERS)
    gtk_file_chooser_set_action(dialog_, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
  if (options & OPTION_MULTI_SELECT)
    gtk_file_chooser_set_select_multiple(dialog_, true);
  if (options & OPTION_SHOW_HIDDEN)
    gtk_file_chooser_set_show_hidden(dialog_, true);
}

void FileDialog::SetFilters(const std::vector<Filter>& filters) {
  for (const Filter& filter : filters) {
    GtkFileFilter* gtk_filter = gtk_file_filter_new();

    for (const std::string& extension : std::get<1>(filter)) {
      std::string ext = "." + extension;
      gtk_file_filter_add_custom(
          gtk_filter,
          GTK_FILE_FILTER_FILENAME,
          reinterpret_cast<GtkFileFilterFunc>(FileFilterCaseInsensitive),
          g_strdup(ext.c_str()),
          g_free);
    }

    gtk_file_filter_set_name(gtk_filter, std::get<0>(filter).c_str());
    gtk_file_chooser_add_filter(dialog_, gtk_filter);
  }
}

base::FilePath FileDialog::AddExtensionForFilename(const char* filename) const {
  base::FilePath path(filename);
  GtkFileFilter* selected_filter = gtk_file_chooser_get_filter(dialog_);
  if (!selected_filter)
    return path;

  GSList* filters = gtk_file_chooser_list_filters(dialog_);
  size_t i = g_slist_index(filters, selected_filter);
  g_slist_free(filters);
  if (i >= filters_.size())
    return path;

  const auto& extensions = std::get<1>(filters_[i]);
  for (const auto& extension : extensions) {
    if (extension == "*" ||
        base::EndsWith(path.value(), "." + extension,
                       base::CompareCase::INSENSITIVE_ASCII))
      return path;
  }

  return path.ReplaceExtension(extensions[0]);
}

}  // namespace nu
