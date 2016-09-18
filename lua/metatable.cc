// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/metatable.h"

namespace lua {

namespace internal {

namespace {

const char* kRefPtrWrapperTableName = "yue.internal.refptrwrappertable";

}  // namespace

// static
bool RefPtrWrapperBase::Push(State* state, void* ptr) {
  int top = GetTop(state);
  luaL_getmetatable(state, kRefPtrWrapperTableName);
  if (GetType(state, -1) != LuaType::Table) {
    SetTop(state, top);
    return false;
  }
  RawGet(state, -1, LightUserData(ptr));
  if (GetType(state, -1) != LuaType::UserData) {
    SetTop(state, top);
    return false;
  }
  lua_remove(state, -2);
  return true;
}

RefPtrWrapperBase::RefPtrWrapperBase(State* state, void* ptr) {
  DCHECK_EQ(GetType(state, -1), LuaType::UserData);
  StackAutoReset reset(state);
  PushWeakTable(state, kRefPtrWrapperTableName, "v");
  RawSet(state, -1, LightUserData(ptr), ValueOnStack(state, -2));
}

int DefaultPropertyLookup(State* state) {
  DCHECK_EQ(GetType(state, 1), LuaType::UserData);
  lua_getmetatable(state, 1);
  DCHECK_EQ(GetType(state, 3), LuaType::Table);
  lua_pushvalue(state, 2);
  lua_gettable(state, 3);
  return 1;
}

}  // namespace internal

}  // namespace lua
