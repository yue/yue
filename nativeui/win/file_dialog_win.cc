// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/file_dialog_win.h"

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "nativeui/win/window_win.h"

namespace nu {

FileDialogImpl::FileDialogImpl(ComPtr<IFileDialog>&& ptr) : dialog_(ptr) {
  // Default filter.
  COMDLG_FILTERSPEC spec = { L"All Files (*.*)", L"*.*" };
  filterspec_.push_back(spec);
  dialog_->SetFileTypes(static_cast<UINT>(filterspec_.size()),
                        filterspec_.data());
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

void FileDialogImpl::SetOptions(int options) {
  dialog_->SetOptions(options);
}

int FileDialogImpl::GetOptions() const {
  int options = 0;
  dialog_->GetOptions(reinterpret_cast<FILEOPENDIALOGOPTIONS*>(&options));
  return options;
}

void FileDialogImpl::SetFilters(
    const std::vector<FileDialog::Filter>& filters) {
  ConvertFilters(filters);
  dialog_->SetFileTypes(static_cast<UINT>(filterspec_.size()),
                        filterspec_.data());

  // By default, *.* will be added to the file name if file type is "*.*". In
  // Electron, we disable it to make a better experience.
  //
  // From MSDN: https://msdn.microsoft.com/en-us/library/windows/desktop/
  // bb775970(v=vs.85).aspx
  //
  // If SetDefaultExtension is not called, the dialog will not update
  // automatically when user choose a new file type in the file dialog.
  //
  // We set file extension to the first none-wildcard extension to make
  // sure the dialog will update file extension automatically.
  for (size_t i = 0; i < filterspec_.size(); i++) {
    const COMDLG_FILTERSPEC& spec = filterspec_[i];
    if (std::wstring(spec.pszSpec) != L"*.*") {
      // SetFileTypeIndex is regarded as one-based index.
      dialog_->SetFileTypeIndex(static_cast<UINT>(i + 1));
      dialog_->SetDefaultExtension(spec.pszSpec);
      break;
    }
  }
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

void FileDialogImpl::ConvertFilters(
    const std::vector<FileDialog::Filter>& filters) {
  buffer_.clear();
  filterspec_.clear();

  if (filters.empty()) {
    COMDLG_FILTERSPEC spec = { L"All Files (*.*)", L"*.*" };
    filterspec_.push_back(spec);
    return;
  }

  buffer_.reserve(filters.size() * 2);
  for (const FileDialog::Filter& filter : filters) {
    COMDLG_FILTERSPEC spec;
    buffer_.push_back(base::UTF8ToWide(std::get<0>(filter)));
    spec.pszName = buffer_.back().c_str();

    std::vector<std::string> extensions(std::get<1>(filter));
    for (std::string& extension : extensions)
      extension.insert(0, "*.");
    buffer_.push_back(base::UTF8ToWide(base::JoinString(extensions, ";")));
    spec.pszSpec = buffer_.back().c_str();

    filterspec_.push_back(spec);
  }
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
  dialog_->SetTitle(base::UTF8ToWide(title));
}

void FileDialog::SetButtonLabel(const std::string& label) {
  dialog_->SetButtonLabel(base::UTF8ToWide(label));
}

void FileDialog::SetFilename(const std::string& filename) {
  dialog_->SetFilename(base::UTF8ToWide(filename));
}

void FileDialog::SetFolder(const base::FilePath& folder) {
  dialog_->SetFolder(folder.value());
}

void FileDialog::SetOptions(int options) {
  int winops = dialog_->GetOptions();
  if (options & OPTION_PICK_FOLDERS)
    winops |= FOS_PICKFOLDERS;
  if (options & OPTION_MULTI_SELECT)
    winops |= FOS_ALLOWMULTISELECT;
  if (options & OPTION_SHOW_HIDDEN)
    winops |= FOS_FORCESHOWHIDDEN;
  dialog_->SetOptions(winops);
}

void FileDialog::SetFilters(const std::vector<Filter>& filters) {
  dialog_->SetFilters(filters);
}

}  // namespace nu
