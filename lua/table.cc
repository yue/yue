// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/table.h"

namespace lua {

namespace {

const char* kCustomDataTableName = "yue.internal.customdatatable";

}  // namespace

void PushCustomDataTable(State* state, int key) {
  key = AbsIndex(state, key);
  PushWeakTable(state, kCustomDataTableName, "k");
  RawGetOrCreateTable(state, -1, ValueOnStack(state, key));
  lua_remove(state, -2);
}

void PushRefsTable(State* state, const char* name, int owner) {
  PushCustomDataTable(state, owner);
  RawGetOrCreateTable(state, -1, name);
  lua_remove(state, -2);
}

}  // namespace lua
