// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/app.h"

#include <utility>

#include "base/base_paths.h"
#include "base/path_service.h"
#include "nativeui/menu_bar.h"
#include "nativeui/state.h"

namespace nu {

// static
App* App::GetCurrent() {
  return State::GetCurrent()->GetApp();
}

App::App() : weak_factory_(this) {
}

App::~App() = default;

void App::SetName(std::string name) {
  name_override_.emplace(std::move(name));
}

std::string App::GetName() const {
  if (name_override_)
    return *name_override_;
  if (!cached_name_) {
    std::string name;
    if (!PlatformGetName(&name)) {
      base::FilePath path;
      if (base::PathService::Get(base::FILE_EXE, &path))
        name = path.BaseName().RemoveExtension().AsUTF8Unsafe();
      else
        name = "Yue";
    }
    cached_name_.emplace(std::move(name));
  }
  return *cached_name_;
}

}  // namespace nu
