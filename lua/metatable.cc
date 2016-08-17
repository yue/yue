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
void PointerWrapperBase::Push(State* state, void* ptr) {
  luaL_getmetatable(state, kPointerWrapperTableName);
  DCHECK_EQ(GetType(state, -1), LuaType::Table);
  RawGet(state, -1, LightUserData(ptr));
  lua_remove(state, -2);
  DCHECK_EQ(GetType(state, -1), LuaType::UserData);
}

// static
int PointerWrapperBase::OnGC(State* state) {
  auto* self = static_cast<PointerWrapperBase*>(lua_touserdata(state, 1));
  self->~PointerWrapperBase();
  return 0;
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

PointerWrapperBase::~PointerWrapperBase() {
}

}  // namespace internal

}  // namespace lua
