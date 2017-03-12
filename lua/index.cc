// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/index.h"

namespace lua {

namespace internal {

int DefaultPropertyLookup(State* state) {
  DCHECK_EQ(GetType(state, 1), LuaType::UserData);
  GetMetaTable(state, 1);  // self
  int metatable = AbsIndex(state, -1);
  DCHECK_EQ(GetType(state, metatable), LuaType::Table);
  Push(state, ValueOnStack(state, 2));  // key
  internal::UnsafeGet(state, metatable);
  return 1;
}

int MemberLookup(State* state) {
  int members = lua_upvalueindex(1);
  RawGet(state, members, ValueOnStack(state, 2));
  if (GetType(state, -1) != LuaType::Nil) {
    auto* holder = static_cast<MemberHolderBase*>(lua_touserdata(state, -1));
    DCHECK(holder);
    return holder->Index(state);
  }
  return DefaultPropertyLookup(state);
}

int MemberAssign(State* state) {
  int members = lua_upvalueindex(1);
  RawGet(state, members, ValueOnStack(state, 2));
  if (GetType(state, -1) != LuaType::Nil) {
    auto* holder = static_cast<MemberHolderBase*>(lua_touserdata(state, -1));
    DCHECK(holder);
    return holder->NewIndex(state);
  }
  return 0;
}

}  // namespace internal

}  // namespace lua
