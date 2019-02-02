// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/file_open_dialog.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/foundation_util.h"
#include "base/strings/sys_string_conversions.h"

namespace nu {

FileOpenDialog::FileOpenDialog() : FileDialog([NSOpenPanel openPanel]) {
}

FileOpenDialog::~FileOpenDialog() {
}

std::vector<base::FilePath> FileOpenDialog::GetResults() const {
  std::vector<base::FilePath> results;
  NSArray* urls = [static_cast<NSOpenPanel*>(GetNative()) URLs];
  for (NSURL* url in urls) {
    if ([url isFileURL])
      results.push_back(base::mac::NSStringToFilePath([url path]));
  }
  return results;
}

}  // namespace nu
