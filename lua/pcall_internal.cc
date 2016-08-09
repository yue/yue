// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "lua/pcall_internal.h"

namespace lua {

namespace internal {

CallbackHolderBase::CallbackHolderBase(State* state) {
  lua_newtable(state);
  lua_pushcfunction(state, &OnGC);
  lua_setfield(state, -2, "__gc");
  lua_setmetatable(state, -2);
}

// static
int CallbackHolderBase::OnGC(State* state) {
  auto* self = static_cast<CallbackHolderBase*>(lua_touserdata(state, 1));
  self->~CallbackHolderBase();
  return 0;
}

}  // namespace internal

}  // namespace lua
