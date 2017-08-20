// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_FILE_SAVE_DIALOG_WIN_H_
#define NATIVEUI_WIN_FILE_SAVE_DIALOG_WIN_H_

#include "nativeui/file_save_dialog.h"
#include "nativeui/win/file_dialog_win.h"

namespace nu {

class FileSaveDialogImpl : public FileDialogImpl {
 public:
  FileSaveDialogImpl();
  ~FileSaveDialogImpl() override;

 private:
  ComPtr<IFileSaveDialog> Initialize() const;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_FILE_SAVE_DIALOG_WIN_H_
