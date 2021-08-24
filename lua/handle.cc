// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/handle.h"

#include "lua/table.h"

namespace lua {

namespace {

const char* kWeakTableName = "yue.internal.weaktable";

}  // namespace

Persistent::Persistent(State* state, int index)
    : Handle(state) {
  lua::Push(state, ValueOnStack(state, index));
  ref_ = luaL_ref(state, LUA_REGISTRYINDEX);
}

Persistent::~Persistent() {
  luaL_unref(state(), LUA_REGISTRYINDEX, ref_);
}

void Persistent::Push() const {
  lua_rawgeti(state(), LUA_REGISTRYINDEX, ref_);
}

Weak::Weak(State* state, int index)
    : Handle(state) {
  index = AbsIndex(state, index);
  StackAutoReset reset(state);
  PushWeakTable(state, kWeakTableName, "v");
  RawSet(state, -1, static_cast<void*>(this), ValueOnStack(state, index));
}

Weak::~Weak() {
  StackAutoReset reset(state());
  luaL_getmetatable(state(), kWeakTableName);
  RawSet(state(), -1, static_cast<void*>(this), nullptr);
}

void Weak::Push() const {
  luaL_getmetatable(state(), kWeakTableName);
  DCHECK_EQ(GetType(state(), -1), LuaType::Table);
  RawGet(state(), -1, static_cast<const void*>(this));
  lua_remove(state(), -2);
}

}  // namespace lua
