// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/metatable.h"

namespace lua {

namespace internal {

namespace {

const char* kPointerWrapperTableName = "yue.internal.pointerwrappertable";

}  // namespace

// static
bool PointerWrapperBase::Push(State* state, void* ptr) {
  int top = GetTop(state);
  luaL_getmetatable(state, kPointerWrapperTableName);
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

PointerWrapperBase::PointerWrapperBase(State* state, void* ptr) {
  DCHECK_EQ(GetType(state, -1), LuaType::UserData);
  StackAutoReset reset(state);
  if (luaL_newmetatable(state, kPointerWrapperTableName) == 1) {
    PushNewTable(state, 0, 1);
    RawSet(state, -1, "__mode", "v");
    SetMetaTable(state, -2);
  }
  RawSet(state, -1, LightUserData(ptr), ValueOnStack(state, -2));
}

}  // namespace internal

}  // namespace lua
