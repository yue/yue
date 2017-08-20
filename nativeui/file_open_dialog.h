// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_FILE_OPEN_DIALOG_H_
#define NATIVEUI_FILE_OPEN_DIALOG_H_

#include <vector>

#include "nativeui/file_dialog.h"

namespace nu {

class NATIVEUI_EXPORT FileOpenDialog : public FileDialog {
 public:
  FileOpenDialog();

  std::vector<base::FilePath> GetResults() const;

 protected:
  ~FileOpenDialog() override;
};

}  // namespace nu

#endif  // NATIVEUI_FILE_OPEN_DIALOG_H_
