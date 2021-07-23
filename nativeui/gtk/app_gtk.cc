// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/app.h"

#include "base/base_paths.h"
#include "base/path_service.h"

namespace nu {

std::string App::PlatformGetName() const {
  base::FilePath path;
  if (base::PathService::Get(base::FILE_EXE, &path))
    return path.BaseName().RemoveExtension().value();
  return "Yue";
}

}  // namespace nu
