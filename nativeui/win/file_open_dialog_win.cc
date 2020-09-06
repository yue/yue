// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/file_open_dialog_win.h"

#include <utility>

namespace nu {

FileOpenDialogImpl::FileOpenDialogImpl() : FileDialogImpl(Initialize()) {
}

FileOpenDialogImpl::~FileOpenDialogImpl() {
}

std::vector<base::FilePath> FileOpenDialogImpl::GetResults() const {
  ComPtr<IFileOpenDialog> dialog = Get<IFileOpenDialog>();
  ComPtr<IShellItemArray> items;
  if (FAILED(dialog->GetResults(&items)))
    return {};
  DWORD count = 0;
  if (FAILED(items->GetCount(&count)))
    return {};
  std::vector<base::FilePath> results;
  results.reserve(count);
  for (DWORD i = 0; i < count; i++) {
     ComPtr<IShellItem> item;
     if (FAILED(items->GetItemAt(i, &item)))
       continue;
     results.push_back(base::FilePath(GetPathFromItem(item)));
  }
  return results;
}

ComPtr<IFileOpenDialog> FileOpenDialogImpl::Initialize() const {
  ComPtr<IFileOpenDialog> ptr;
  ::CoCreateInstance(CLSID_FileOpenDialog,
                     nullptr,
                     CLSCTX_INPROC_SERVER,
                     IID_PPV_ARGS(&ptr));
  return ptr;
}

///////////////////////////////////////////////////////////////////////////////
// Public FileOpenDialog API implementation.

FileOpenDialog::FileOpenDialog() : FileDialog(new FileOpenDialogImpl) {
}

FileOpenDialog::~FileOpenDialog() {
}

std::vector<base::FilePath> FileOpenDialog::GetResults() const {
  return static_cast<FileOpenDialogImpl*>(GetNative())->GetResults();
}

}  // namespace nu
