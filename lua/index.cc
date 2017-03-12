// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/index.h"

namespace lua {

int DefaultPropertyLookup(State* state) {
  DCHECK_EQ(GetType(state, 1), LuaType::UserData);
  lua_getmetatable(state, 1);
  DCHECK_EQ(GetType(state, 3), LuaType::Table);
  lua_pushvalue(state, 2);
  lua_gettable(state, 3);
  return 1;
}

}  // namespace lua
