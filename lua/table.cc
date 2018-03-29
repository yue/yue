// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/table.h"

namespace lua {

namespace {

const char* kCustomDataTableName = "yue.internal.customdatatable";

}  // namespace

void PushCustomDataTable(State* state, int key) {
  int top = GetTop(state);
  key = AbsIndex(state, key);
  PushWeakTable(state, kCustomDataTableName, "k");
  RawGet(state, -1, ValueOnStack(state, key));
  if (GetType(state, -1) == LuaType::Nil) {
    NewTable(state);
    RawSet(state, top + 1, ValueOnStack(state, key), ValueOnStack(state, -1));
  }
  lua_insert(state, top + 1);
  SetTop(state, top + 1);
}

void PushRefsTable(State* state, const char* name, int owner) {
  PushCustomDataTable(state, owner);
  int custom_data = AbsIndex(state, -1);
  RawGet(state, custom_data, name);
  if (GetType(state, -1) != LuaType::Table) {
    PopAndIgnore(state, 1);
    NewTable(state, 0, 1);
    RawSet(state, custom_data, name, ValueOnStack(state, -1));
  }
  lua_insert(state, custom_data);
  PopAndIgnore(state, 1);
}

}  // namespace lua
