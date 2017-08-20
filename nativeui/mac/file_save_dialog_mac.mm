// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/file_save_dialog.h"

#import <Cocoa/Cocoa.h>

namespace nu {

FileSaveDialog::FileSaveDialog() : FileDialog([NSSavePanel savePanel]) {
}

FileSaveDialog::~FileSaveDialog() {
}

}  // namespace nu
