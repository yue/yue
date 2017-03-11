// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/metatable.h"

namespace lua {

namespace internal {

namespace {

const char* kWrappaerTablename = "yue.internal.wrappertable";

}  // namespace

bool WrapperTableGet(State* state, void* key) {
  int top = GetTop(state);
  PushWeakTable(state, kWrappaerTablename, "v");
  RawGet(state, -1, key);
  if (GetType(state, -1) == LuaType::Nil) {
    SetTop(state, top);
    return false;
  }
  lua_remove(state, -2);
  return true;
}

void WrapperTableSet(State* state, void* key, int index) {
  index = AbsIndex(state, index);
  StackAutoReset reset(state);
  PushWeakTable(state, kWrappaerTablename, "v");
  RawSet(state, -1, key, ValueOnStack(state, index));
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
