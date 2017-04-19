// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/index.h"

namespace lua {

namespace internal {

namespace {

const char* kCacheTableName = "yue.internal.cachetable";

}  // namespace

void PushCacheTable(State* state, int key) {
  int top = GetTop(state);
  PushWeakTable(state, kCacheTableName, "k");
  RawGet(state, -1, ValueOnStack(state, key));
  if (GetType(state, -1) == LuaType::Nil) {
    NewTable(state);
    RawSet(state, top + 1, ValueOnStack(state, key), ValueOnStack(state, -1));
  }
  lua_insert(state, top + 1);
  SetTop(state, top + 1);
}

}  // namespace internal

}  // namespace lua
