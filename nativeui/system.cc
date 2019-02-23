// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/system.h"

#include "base/files/file_path.h"
#include "base/path_service.h"
#include "nativeui/state.h"

#if defined(OS_LINUX)
#include "base/environment.h"
#include "base/nix/xdg_util.h"
#endif

namespace nu {

base::FilePath System::GetPath(Path name) {
  base::FilePath ret;
  switch (name) {
    case Path::AppData:
#if defined(OS_MACOSX) || defined(OS_WIN)
      base::PathService::Get(base::DIR_APP_DATA, &ret);
#else
      {
        std::unique_ptr<base::Environment> env(base::Environment::Create());
        ret = base::nix::GetXDGDirectory(
            env.get(),
            base::nix::kXdgConfigHomeEnvVar,
            base::nix::kDotConfigDir);
      }
#endif
      break;
    case Path::Cache:
#if defined(OS_POSIX)
      base::PathService::Get(base::DIR_CACHE, &ret);
#else
      base::PathService::Get(base::DIR_LOCAL_APP_DATA, &ret);
#endif
      break;
    case Path::Home:
      base::PathService::Get(base::DIR_HOME, &ret);
      break;
    case Path::Desktop:
      base::PathService::Get(base::DIR_USER_DESKTOP, &ret);
      break;
    default:
      NOTREACHED() << "Unknown path name: " << static_cast<int>(name);
      return ret;
  }
  return ret;
}

Font* System::GetDefaultFont() {
  return State::GetCurrent()->GetDefaultFont();
}

}  // namespace nu
