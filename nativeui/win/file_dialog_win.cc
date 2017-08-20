// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/file_dialog_win.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/win/window_win.h"

namespace nu {

FileDialogImpl::FileDialogImpl(ComPtr<IFileDialog>&& ptr) : dialog_(ptr) {
}

FileDialogImpl::~FileDialogImpl() {
}

std::wstring FileDialogImpl::GetResult() const {
  ComPtr<IShellItem> result;
  if (FAILED(dialog_->GetResult(&result)))
    return std::wstring();
  return GetPathFromItem(result);
}

bool FileDialogImpl::Run() {
  return SUCCEEDED(dialog_->Show(NULL));
}

bool FileDialogImpl::RunForWindow(WindowImpl* window) {
  return SUCCEEDED(dialog_->Show(window->hwnd()));
}

void FileDialogImpl::SetTitle(const std::wstring& title) {
  dialog_->SetTitle(title.c_str());
}

void FileDialogImpl::SetButtonLabel(const std::wstring& label) {
  dialog_->SetOkButtonLabel(label.c_str());
}

void FileDialogImpl::SetFilename(const std::wstring& filename) {
  dialog_->SetFileName(filename.c_str());
}

void FileDialogImpl::SetFolder(const std::wstring& folder) {
  ComPtr<IShellItem> item;
  if (SUCCEEDED(::SHCreateItemFromParsingName(folder.c_str(), nullptr,
                                              IID_PPV_ARGS(&item))))
    dialog_->SetFolder(item.Get());
}

std::wstring FileDialogImpl::GetPathFromItem(
    const ComPtr<IShellItem>& item) const {
  wchar_t* name = nullptr;
  item->GetDisplayName(SIGDN_FILESYSPATH, &name);
  if (!name)
    return std::wstring();
  std::wstring filename = name;
  ::CoTaskMemFree(name);
  return filename;
}

///////////////////////////////////////////////////////////////////////////////
// Public FileDialog API implementation.

FileDialog::FileDialog(NativeFileDialog dialog) : dialog_(dialog) {
}

FileDialog::~FileDialog() {
  delete dialog_;
}

base::FilePath FileDialog::GetResult() const {
  return base::FilePath(dialog_->GetResult());
}

bool FileDialog::Run() {
  return dialog_->Run();
}

bool FileDialog::RunForWindow(Window* window) {
  return dialog_->RunForWindow(window->GetNative());
}

void FileDialog::SetTitle(const std::string& title) {
  dialog_->SetTitle(base::UTF8ToUTF16(title));
}

void FileDialog::SetButtonLabel(const std::string& label) {
  dialog_->SetButtonLabel(base::UTF8ToUTF16(label));
}

void FileDialog::SetFilename(const std::string& filename) {
  dialog_->SetFilename(base::UTF8ToUTF16(filename));
}

void FileDialog::SetFolder(const base::FilePath& folder) {
  dialog_->SetFolder(folder.value());
}

}  // namespace nu
