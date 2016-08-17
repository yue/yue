// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/callback_internal.h"

#include "lua/table.h"

namespace lua {

namespace internal {

CallbackHolderBase::CallbackHolderBase(State* state) {
  PushNewTable(state);
  RawSet(state, -1, "__gc", CFunction(&OnGC));
  SetMetaTable(state, -2);
}

// static
int CallbackHolderBase::OnGC(State* state) {
  auto* self = static_cast<CallbackHolderBase*>(lua_touserdata(state, 1));
  self->~CallbackHolderBase();
  return 0;
}

}  // namespace internal

}  // namespace lua
