// Copyright 2021 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/gtk/util/desktop_file.h"

#include <glib.h>

#include "base/environment.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/nix/xdg_util.h"
#include "base/strings/string_tokenizer.h"

namespace nu {

namespace {

const char kDesktopEntry[] = "Desktop Entry";

}  // namespace

base::FilePath GetDataWriteLocation(base::Environment* env) {
  return base::nix::GetXDGDirectory(env, "XDG_DATA_HOME", ".local/share");
}

std::vector<base::FilePath> GetDataSearchLocations(base::Environment* env) {
  std::vector<base::FilePath> search_paths;
  base::FilePath write_location = GetDataWriteLocation(env);
  search_paths.push_back(write_location);

  std::string xdg_data_dirs;
  if (env->GetVar("XDG_DATA_DIRS", &xdg_data_dirs) && !xdg_data_dirs.empty()) {
    base::StringTokenizer tokenizer(xdg_data_dirs, ":");
    while (tokenizer.GetNext()) {
      search_paths.emplace_back(tokenizer.token_piece());
    }
  } else {
    search_paths.push_back(base::FilePath("/usr/local/share"));
    search_paths.push_back(base::FilePath("/usr/share"));
  }

  return search_paths;
}

bool GetDesktopFileContents(base::Environment* env,
                            base::StringPiece desktop_filename,
                            std::string* output) {
  std::vector<base::FilePath> search_paths = GetDataSearchLocations(env);

  for (std::vector<base::FilePath>::const_iterator i = search_paths.begin();
       i != search_paths.end(); ++i) {
    base::FilePath path = i->Append("applications").Append(desktop_filename);
    VLOG(1) << "Looking for desktop file in " << path.value();
    if (base::PathExists(path)) {
      VLOG(1) << "Found desktop file at " << path.value();
      return base::ReadFileToString(path, output);
    }
  }

  return false;
}

bool GetNameFromDesktopFile(base::StringPiece contents, std::string* name) {
  // An empty file causes a crash with glib <= 2.32, so special case here.
  if (contents.empty())
    return false;

  GKeyFile* key_file = g_key_file_new();
  GError* err = NULL;
  if (!g_key_file_load_from_data(key_file, contents.data(), contents.size(),
                                 G_KEY_FILE_NONE, &err)) {
    LOG(WARNING) << "Unable to read desktop file: " << err->message;
    g_error_free(err);
    g_key_file_free(key_file);
    return false;
  }

  char* name_c_string = g_key_file_get_string(key_file, kDesktopEntry,
                                              "Name", &err);
  if (!name_c_string) {
    g_error_free(err);
    return false;
  }

  *name = name_c_string;
  g_free(name_c_string);
  g_key_file_free(key_file);
  return true;
}

}  // namespace nu
