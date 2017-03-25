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
  PushWeakTable(state, kWeakTableName, "v");
  lua::Push(state, ValueOnStack(state, index));
  return luaL_ref(state, -2);
}

void PushWeakReference(State* state, int ref) {
  luaL_getmetatable(state, kWeakTableName);
  DCHECK_EQ(GetType(state, -1), LuaType::Table);
  lua_rawgeti(state, -1, ref);
  lua_remove(state, -2);
}

bool WeakReferenceExists(State* state, int ref) {
  StackAutoReset reset(state);
  luaL_getmetatable(state, kWeakTableName);
  DCHECK_EQ(GetType(state, -1), LuaType::Table);
  lua_rawgeti(state, -1, ref);
  return GetType(state, -1) != LuaType::Nil;
}

}  // namespace lua
