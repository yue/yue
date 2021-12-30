// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_FILE_DIALOG_H_
#define NATIVEUI_FILE_DIALOG_H_

#include <string>
#include <tuple>
#include <vector>

#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"

namespace nu {

class Window;

class NATIVEUI_EXPORT FileDialog : public base::RefCounted<FileDialog> {
 public:
  FileDialog& operator=(const FileDialog&) = delete;
  FileDialog(const FileDialog&) = delete;

  // Possible options that can be OR-ed.
  enum Option {
    OPTION_PICK_FOLDERS = 1 << 0,
    OPTION_MULTI_SELECT = 1 << 1,
    OPTION_SHOW_HIDDEN = 1 << 2,
  };

  // <description, extensions>
  using Filter = std::tuple<std::string, std::vector<std::string>>;

  base::FilePath GetResult() const;
  bool Run();
  bool RunForWindow(Window* window);
  void SetTitle(const std::string& title);
  void SetButtonLabel(const std::string& label);
  void SetFilename(const std::string& filename);
  void SetFolder(const base::FilePath& folder);
  void SetOptions(int options);
  void SetFilters(const std::vector<Filter>& filters);
  std::vector<Filter> GetFilters() const { return filters_; }

  NativeFileDialog GetNative() const { return dialog_; }

 protected:
  explicit FileDialog(NativeFileDialog dialog);
  virtual ~FileDialog();

#if defined(OS_LINUX)
  // Add extensions to the filename returned by file chooser.
  base::FilePath AddExtensionForFilename(const char* filename) const;
#endif

 private:
  friend class base::RefCounted<FileDialog>;

  std::vector<Filter> filters_;
  NativeFileDialog dialog_;
};

}  // namespace nu

#endif  // NATIVEUI_FILE_DIALOG_H_
