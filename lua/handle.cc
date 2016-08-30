// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/handle.h"

#include "lua/table.h"

namespace lua {

namespace {

const char* kWeakTableName = "yue.internal.weaktable";

}  // namespace

int CreateWeakReference(State* state, int index) {
  index = AbsIndex(state, index);
  StackAutoReset reset(state);
  if (luaL_newmetatable(state, kWeakTableName) == 1) {
    PushNewTable(state, 0, 1);
    RawSet(state, -1, "__mode", "v");
    SetMetaTable(state, -2);
  }
  lua::Push(state, ValueOnStack(state, index));
  return luaL_ref(state, -2);
}

void PushWeakReference(State* state, int ref) {
  luaL_getmetatable(state, kWeakTableName);
  DCHECK_EQ(GetType(state, -1), LuaType::Table);
  lua_rawgeti(state, -1, ref);
  lua_remove(state, -2);
}

}  // namespace lua
