// Copyright 2021 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_GTK_UTIL_DESKTOP_FILE_H_
#define NATIVEUI_GTK_UTIL_DESKTOP_FILE_H_

#include <string>
#include <vector>

#include "base/files/file_path.h"

namespace base {
class Environment;
}

namespace nu {

// Get the path to write user-specific application data files to, as specified
// in the XDG Base Directory Specification:
// http://standards.freedesktop.org/basedir-spec/latest/
base::FilePath GetDataWriteLocation(base::Environment* env);

// Get the list of paths to search for application data files, in order of
// preference, as specified in the XDG Base Directory Specification:
// http://standards.freedesktop.org/basedir-spec/latest/
// Called on the FILE thread.
std::vector<base::FilePath> GetDataSearchLocations(base::Environment* env);

// Returns the contents of an existing .desktop file installed in the system.
// Searches the "applications" subdirectory of each XDG data directory for a
// file named |desktop_filename|. If the file is found, populates |output| with
// its contents and returns true. Else, returns false.
bool GetDesktopFileContents(base::Environment* env,
                            base::StringPiece desktop_filename,
                            std::string* output);

// Returns the name of app from the desktop file.
bool GetNameFromDesktopFile(base::StringPiece contents, std::string* name);

}  // namespace nu

#endif  // NATIVEUI_GTK_UTIL_DESKTOP_FILE_H_
