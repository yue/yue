// Copyright 2017 Cheng Zhao. All rights reserved.
// Copyright 2013 GitHub, Inc.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/file_dialog.h"

#import <Cocoa/Cocoa.h>
#import <CoreServices/CoreServices.h>

#include "base/mac/foundation_util.h"
#include "base/mac/scoped_cftyperef.h"
#include "base/strings/sys_string_conversions.h"
#include "nativeui/window.h"

namespace nu {

FileDialog::FileDialog(NativeFileDialog dialog) : dialog_(dialog) {
  [dialog_ retain];  // dialog is an autorelease object
  dialog_.canSelectHiddenExtension = YES;
  dialog_.allowsOtherFileTypes = YES;
  dialog_.extensionHidden = NO;
}

FileDialog::~FileDialog() {
  [dialog_ release];
}

base::FilePath FileDialog::GetResult() const {
  return base::mac::NSStringToFilePath([[dialog_ URL] path]);
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

void FileDialog::SetOptions(int options) {
  if ([dialog_ class] == [NSOpenPanel class]) {
    NSOpenPanel* panel = static_cast<NSOpenPanel*>(dialog_);
    if (options & OPTION_PICK_FOLDERS) {
      panel.canChooseDirectories = YES;
      // Keep consistency with other platforms.
      panel.canChooseFiles = NO;
    }
    if (options & OPTION_MULTI_SELECT)
      panel.allowsMultipleSelection = YES;
  }
  if (options & OPTION_SHOW_HIDDEN)
    dialog_.showsHiddenFiles = YES;
}

void FileDialog::SetFilters(const std::vector<Filter>& filters) {
  if (!filters.empty())
    dialog_.allowsOtherFileTypes = NO;

  NSMutableSet* file_type_set = [NSMutableSet set];
  for (const Filter& filter : filters) {
    for (const std::string& extension : std::get<1>(filter)) {
      // If we meet a '*' file extension, we allow all the file types.
      if (extension == "*") {
        dialog_.allowsOtherFileTypes = YES;
        return;
      }
      base::ScopedCFTypeRef<CFStringRef> ext_cf(
          base::SysUTF8ToCFStringRef(extension));
      [file_type_set addObject:base::mac::CFToNSCast(ext_cf.get())];
    }
  }

  // Passing empty array to setAllowedFileTypes will cause exception.
  NSArray* file_types = nil;
  if ([file_type_set count])
    file_types = [file_type_set allObjects];

  dialog_.allowedFileTypes = file_types;
}

}  // namespace nu
