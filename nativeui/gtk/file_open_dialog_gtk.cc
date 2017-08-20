// Copyright 2017 Cheng Zhao. All rights reserved.
// Copyright 2014 GitHub, Inc.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/file_open_dialog.h"

#include <gtk/gtk.h>

namespace nu {

FileOpenDialog::FileOpenDialog()
    : FileDialog(GTK_FILE_CHOOSER(gtk_file_chooser_dialog_new(
                     nullptr, nullptr,
                     GTK_FILE_CHOOSER_ACTION_OPEN,
                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                     GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                     nullptr))) {
}

FileOpenDialog::~FileOpenDialog() {
}

std::vector<base::FilePath> FileOpenDialog::GetResults() const {
  std::vector<base::FilePath> results;
  GSList* filenames = gtk_file_chooser_get_filenames(GetNative());
  for (GSList* iter = filenames; iter; iter = g_slist_next(iter)) {
    auto path = AddExtensionForFilename(static_cast<char*>(iter->data));
    g_free(iter->data);
    results.push_back(path);
  }
  g_slist_free(filenames);
  return results;
}

}  // namespace nu
