// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/callback.h"

#include "lua/table.h"
#include "lua/user_data.h"

namespace lua {

namespace internal {

CallbackHolderBase::CallbackHolderBase(State* state) {
  PushNewTable(state);
  RawSet(state, -1, "__gc", CFunction(&OnGC<CallbackHolderBase>));
  SetMetaTable(state, -2);
}

}  // namespace internal

}  // namespace lua
