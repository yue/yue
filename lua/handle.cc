// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/handle.h"

#include "lua/table.h"

namespace lua {

namespace {

const char* kWeakTableName = "yue.internal.weaktable";

}  // namespace

void CreateWeakReference(State* state, void* key, int index) {
  index = AbsIndex(state, index);
  StackAutoReset reset(state);
  PushWeakTable(state, kWeakTableName, "v");
  RawSet(state, -1, key, ValueOnStack(state, index));
}

void PushWeakReference(State* state, void* key) {
  luaL_getmetatable(state, kWeakTableName);
  DCHECK_EQ(GetType(state, -1), LuaType::Table);
  RawGet(state, -1, key);
  lua_remove(state, -2);
}

bool WeakReferenceExists(State* state, void* key) {
  StackAutoReset reset(state);
  luaL_getmetatable(state, kWeakTableName);
  DCHECK_EQ(GetType(state, -1), LuaType::Table);
  RawGet(state, -1, key);
  return GetType(state, -1) != LuaType::Nil;
}

}  // namespace lua
