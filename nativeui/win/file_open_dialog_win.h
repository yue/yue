// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_FILE_OPEN_DIALOG_WIN_H_
#define NATIVEUI_WIN_FILE_OPEN_DIALOG_WIN_H_

#include <vector>

#include "nativeui/file_open_dialog.h"
#include "nativeui/win/file_dialog_win.h"

namespace nu {

class FileOpenDialogImpl : public FileDialogImpl {
 public:
  FileOpenDialogImpl();
  ~FileOpenDialogImpl() override;

  std::vector<base::FilePath> GetResults() const;

 private:
  ComPtr<IFileOpenDialog> Initialize() const;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_FILE_OPEN_DIALOG_WIN_H_
