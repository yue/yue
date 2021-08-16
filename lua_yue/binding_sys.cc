// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua_yue/binding_sys.h"

#include "build/build_config.h"

namespace {

// Determine current platform.
#if defined(OS_WIN)
#define PLATFORM "win32"
#elif defined(OS_LINUX)
#define PLATFORM "linux"
#elif defined(OS_MAC)
#define PLATFORM "macos"
#else
#define PLATFORM "other"
#endif

}  // namespace

extern "C" int luaopen_yue_sys(lua::State* state) {
  lua::NewTable(state);
  lua::RawSet(state, -1, "platform", PLATFORM);
  return 1;
}
