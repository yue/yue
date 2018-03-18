// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/index.h"
#include "lua/metatable.h"

namespace lua {

namespace internal {

namespace {

// Limit for table inheritance chains (to avoid loops).
const int kMaxLoop  = 2000;

const char* kWrapperTableName = "yue.internal.wrappertable";

}  // namespace

bool WrapperTableGet(State* state, void* key) {
  int top = GetTop(state);
  PushWeakTable(state, kWrapperTableName, "v");
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
  PushWeakTable(state, kWrapperTableName, "v");
  RawSet(state, -1, key, ValueOnStack(state, index));
}

int InheritanceChainLookup(State* state) {
  // The first metatable to lookup.
  Push(state, ValueOnStack(state, lua_upvalueindex(1)));
  int metatable = 3;

  for (int loop = 0; loop < kMaxLoop; loop++) {
    // First check current metatable.
    RawGet(state, metatable, ValueOnStack(state, 2));
    if (GetType(state, 4) != LuaType::Nil)
      return 1;

    // Then look up the properties.
    RawGet(state, metatable, "__properties");
    if (GetType(state, 5) != LuaType::Nil) {
      RawGet(state, 5, ValueOnStack(state, 2));
      if (GetType(state, 6) != LuaType::Nil) {
        auto* holder = static_cast<MemberHolderBase*>(lua_touserdata(state, 6));
        SetTop(state, 2);
        return holder->Index(state);
      }
    }
    SetTop(state, 3);

    // Does this metatable have a super class.
    RawGet(state, metatable, "__super");
    if (GetType(state, -1) == LuaType::Nil) {
      // So the property is not found anywhere in the inheiritance chain, lets
      // lookup the custom data table.
      PushCustomDataTable(state, 1);
      RawGet(state, -1, ValueOnStack(state, 2));
      return 1;
    }

    // Lookup in the super class in chain.
    lua_insert(state, 3);
    SetTop(state, 3);
  }

  Push(state, "'__index' chain too long; possible loop");
  lua_error(state);
  return 0;
}

int InheritanceChainAssign(State* state) {
  // The first metatable to lookup.
  Push(state, ValueOnStack(state, lua_upvalueindex(1)));
  int metatable = 4;

  for (int loop = 0; loop < kMaxLoop; loop++) {
    // First look up the properties.
    RawGet(state, metatable, "__properties");
    if (GetType(state, 5) != LuaType::Nil) {
      RawGet(state, 5, ValueOnStack(state, 2));
      if (GetType(state, 6) != LuaType::Nil) {
        auto* holder = static_cast<MemberHolderBase*>(lua_touserdata(state, 6));
        SetTop(state, 3);
        return holder->NewIndex(state);
      }
    }
    SetTop(state, 4);

    // Does this metatable have a super class.
    RawGet(state, metatable, "__super");
    if (GetType(state, -1) == LuaType::Nil) {
      // So the property is not found anywhere in the inheiritance chain, lets
      // lookup the custom data table.
      PushCustomDataTable(state, 1);
      RawSet(state, -1, ValueOnStack(state, 2), ValueOnStack(state, 3));
      Push(state, ValueOnStack(state, 3));
      return 1;
    }

    // Lookup in the super class in chain.
    lua_insert(state, 4);
    SetTop(state, 4);
  }

  Push(state, "'__newindex' chain too long; possible loop");
  lua_error(state);
  return 0;
}

}  // namespace internal

}  // namespace lua
