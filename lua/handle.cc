// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/handle.h"

#include "lua/table.h"

namespace lua {

namespace {

const char* kWeakTableName = "yue.internal.weaktable";

}  // namespace

Weak::Weak(State* state, int index) : state_(state) {
  StackAutoReset reset(state);
  if (luaL_newmetatable(state, kWeakTableName) == 1) {
    PushNewTable(state, 0, 1);
    RawSet(state, -1, "__mode", "v");
    lua::SetMetaTable(state, -2);
  }
  lua::Push(state, ValueOnStack(state, index));
  ref_ = luaL_ref(state, -2);
}

void Weak::Push(State* state) const {
  DCHECK_EQ(state, state_) << "Pushing a handle on wrong thread";
  luaL_getmetatable(state, kWeakTableName);
  lua_rawgeti(state, -1, ref_);
  lua_remove(state, -2);
}

}  // namespace lua
