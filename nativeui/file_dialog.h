// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_FILE_DIALOG_H_
#define NATIVEUI_FILE_DIALOG_H_

#include <string>

#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"

namespace nu {

class Window;

class NATIVEUI_EXPORT FileDialog : public base::RefCounted<FileDialog> {
 public:
  enum Option {
    OPTION_PICK_FOLDERS = 1 << 0,
    OPTION_MULTI_SELECT = 1 << 1,
    OPTION_SHOW_HIDDEN = 1 << 2,
  };

  base::FilePath GetResult() const;
  bool Run();
  bool RunForWindow(Window* window);
  void SetTitle(const std::string& title);
  void SetButtonLabel(const std::string& label);
  void SetFilename(const std::string& filename);
  void SetFolder(const base::FilePath& folder);
  void SetOptions(int options);

  NativeFileDialog GetNative() const { return dialog_; }

 protected:
  explicit FileDialog(NativeFileDialog dialog);
  virtual ~FileDialog();

 private:
  friend class base::RefCounted<FileDialog>;

  NativeFileDialog dialog_;

  DISALLOW_COPY_AND_ASSIGN(FileDialog);
};

}  // namespace nu

#endif  // NATIVEUI_FILE_DIALOG_H_
