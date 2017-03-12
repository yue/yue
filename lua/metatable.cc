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

}  // namespace internal

}  // namespace lua
