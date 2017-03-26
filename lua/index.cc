// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/index.h"

namespace lua {

namespace internal {

// Implement the default __index handler which looks up in the metatable.
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
  // Check the user-added members.
  int datas = lua_upvalueindex(1);
  RawGet(state, datas, ValueOnStack(state, 2));
  if (GetType(state, -1) != LuaType::Nil) {
    return 1;
  }
  // Check the pre-defined members.
  int members = lua_upvalueindex(2);
  RawGet(state, members, ValueOnStack(state, 2));
  if (GetType(state, -1) != LuaType::Nil) {
    auto* holder = static_cast<MemberHolderBase*>(lua_touserdata(state, -1));
    DCHECK(holder);
    return holder->Index(state);
  }
  return DefaultPropertyLookup(state);
}

int MemberAssign(State* state) {
  // Check the pre-defined members.
  int members = lua_upvalueindex(2);
  RawGet(state, members, ValueOnStack(state, 2));
  if (GetType(state, -1) != LuaType::Nil) {
    auto* holder = static_cast<MemberHolderBase*>(lua_touserdata(state, -1));
    DCHECK(holder);
    return holder->NewIndex(state);
  }
  // Set a custom data.
  int datas = lua_upvalueindex(1);
  RawSet(state, datas, ValueOnStack(state, 2), ValueOnStack(state, 3));
  return 1;
}

}  // namespace internal

}  // namespace lua
