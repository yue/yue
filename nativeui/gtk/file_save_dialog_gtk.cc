// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/file_save_dialog.h"

#include <gtk/gtk.h>

namespace nu {

FileSaveDialog::FileSaveDialog()
    : FileDialog(GTK_FILE_CHOOSER(gtk_file_chooser_dialog_new(
                     nullptr, nullptr,
                     GTK_FILE_CHOOSER_ACTION_SAVE,
                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                     GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                     nullptr))) {
  gtk_file_chooser_set_do_overwrite_confirmation(GetNative(), true);
  gtk_file_chooser_set_create_folders(GetNative(), true);
}

FileSaveDialog::~FileSaveDialog() {
}

}  // namespace nu
