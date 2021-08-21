// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/lua.h"
#include "lua/lua_module_export.h"
#include "testing/gtest/include/gtest/gtest.h"

extern "C" LUA_MODULE_EXPORT int luaopen_lua_unittests(lua::State* state) {
  int argc = 1;
  char program[] = "lua";
  char* argv[] = { program };
  ::testing::InitGoogleTest(&argc, argv);

  lua::Push(state, RUN_ALL_TESTS());
  return 1;
}
