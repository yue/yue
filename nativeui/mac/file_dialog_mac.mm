// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/file_dialog.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"
#include "nativeui/window.h"

namespace nu {

FileDialog::FileDialog(NativeFileDialog dialog) : dialog_(dialog) {
  [dialog_ retain];  // dialog is an autorelease object
}

FileDialog::~FileDialog() {
  [dialog_ release];
}

base::FilePath FileDialog::GetResult() const {
  return base::FilePath(base::SysNSStringToUTF8([[dialog_ URL] path]));
}

bool FileDialog::Run() {
  return [dialog_ runModal] == NSFileHandlingPanelOKButton;
}

bool FileDialog::RunForWindow(Window* window) {
  __block int chosen = NSFileHandlingPanelCancelButton;
  [dialog_ beginSheetModalForWindow:window->GetNative()
                 completionHandler:^(NSInteger c) {
    chosen = c;
    [NSApp stopModal];
  }];
  [NSApp runModalForWindow:window->GetNative()];
  return chosen == NSFileHandlingPanelOKButton;
}

void FileDialog::SetTitle(const std::string& title) {
  dialog_.title = base::SysUTF8ToNSString(title);
}

void FileDialog::SetButtonLabel(const std::string& label) {
  dialog_.prompt = base::SysUTF8ToNSString(label);
}

void FileDialog::SetFilename(const std::string& filename) {
  dialog_.nameFieldStringValue = base::SysUTF8ToNSString(filename);
}

void FileDialog::SetFolder(const base::FilePath& folder) {
  dialog_.directoryURL =
      [NSURL fileURLWithPath:base::SysUTF8ToNSString(folder.value())];
}

}  // namespace nu
