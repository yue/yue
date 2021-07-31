// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/app.h"

#include "base/environment.h"
#include "nativeui/gtk/util/desktop_file.h"

namespace nu {

void App::SetID(std::string name) {
  desktop_name_ = std::move(name);
}

std::string App::GetID() const {
  return desktop_name_;
}

bool App::PlatformGetName(std::string* name) const {
  if (desktop_name_.empty())
    return false;
  std::unique_ptr<base::Environment> env = base::Environment::Create();
  std::string contents;
  if (!GetDesktopFileContents(env.get(), desktop_name_ + ".desktop", &contents))
    return false;
  return GetNameFromDesktopFile(contents, name);
}

}  // namespace nu
