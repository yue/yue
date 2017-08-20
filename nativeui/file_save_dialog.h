// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_FILE_SAVE_DIALOG_H_
#define NATIVEUI_FILE_SAVE_DIALOG_H_

#include "nativeui/file_dialog.h"

namespace nu {

class NATIVEUI_EXPORT FileSaveDialog : public FileDialog {
 public:
  FileSaveDialog();

 protected:
  ~FileSaveDialog() override;
};

}  // namespace nu

#endif  // NATIVEUI_FILE_SAVE_DIALOG_H_
