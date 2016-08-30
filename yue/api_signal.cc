// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "yue/api_signal.h"

namespace yue {

namespace {

const char* kMembersMapName = "yue.internal.membersmap";

}  // namespace

// Push a weak table which records the object's members.
void PushObjectMembersTable(lua::State* state, int index) {
  int top = lua::GetTop(state);
  // Get the table for recording objects.
  lua::PushWeakTable(state, kMembersMapName, "k");
  lua::RawGet(state, -1, lua::ValueOnStack(state, index));
  if (lua::GetType(state, -1) != lua::LuaType::Table) {
    // This is the first record.
    lua::PushNewTable(state);
    lua::RawSet(state, -3, lua::ValueOnStack(state, index),
                           lua::ValueOnStack(state, -1));
  }
  // Pop the metatable and keep the signal table.
  lua::Insert(state, top + 1);
  lua::SetTop(state, top + 1);
  DCHECK_EQ(lua::GetType(state, -1), lua::LuaType::Table);
}

}  // namespace yue
