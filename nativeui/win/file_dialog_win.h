// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_FILE_DIALOG_WIN_H_
#define NATIVEUI_WIN_FILE_DIALOG_WIN_H_

#include <shobjidl.h>
#include <wrl/client.h>

#include <string>
#include <utility>
#include <vector>

#include "nativeui/file_dialog.h"

namespace nu {

using Microsoft::WRL::ComPtr;

class FileDialogImpl {
 public:
  virtual ~FileDialogImpl();

  std::wstring GetResult() const;
  bool Run();
  bool RunForWindow(WindowImpl* window);
  void SetTitle(const std::wstring& title);
  void SetButtonLabel(const std::wstring& label);
  void SetFilename(const std::wstring& filename);
  void SetFolder(const std::wstring& folder);
  void SetOptions(int options);
  int GetOptions() const;
  void SetFilters(const std::vector<FileDialog::Filter>& filters);

 protected:
  explicit FileDialogImpl(ComPtr<IFileDialog>&& ptr);

  template<typename T>
  ComPtr<T> Get() const {
    ComPtr<T> ptr;
    dialog_.As<T>(&ptr);
    return std::move(ptr);
  }

  std::wstring GetPathFromItem(const ComPtr<IShellItem>& item) const;
  void ConvertFilters(const std::vector<FileDialog::Filter>& filters);

 private:
  ComPtr<IFileDialog> dialog_;

  std::vector<std::wstring> buffer_;
  std::vector<COMDLG_FILTERSPEC> filterspec_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_FILE_DIALOG_WIN_H_
