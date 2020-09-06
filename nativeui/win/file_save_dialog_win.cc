// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/file_save_dialog_win.h"

#include <utility>

namespace nu {

FileSaveDialogImpl::FileSaveDialogImpl() : FileDialogImpl(Initialize()) {
}

FileSaveDialogImpl::~FileSaveDialogImpl() {
}

ComPtr<IFileSaveDialog> FileSaveDialogImpl::Initialize() const {
  ComPtr<IFileSaveDialog> ptr;
  ::CoCreateInstance(CLSID_FileSaveDialog,
                     nullptr,
                     CLSCTX_INPROC_SERVER,
                     IID_PPV_ARGS(&ptr));
  return ptr;
}

///////////////////////////////////////////////////////////////////////////////
// Public FileSaveDialog API implementation.

FileSaveDialog::FileSaveDialog() : FileDialog(new FileSaveDialogImpl) {
}

FileSaveDialog::~FileSaveDialog() {
}

}  // namespace nu
