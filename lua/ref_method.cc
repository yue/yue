// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/ref_method.h"

namespace lua {

namespace internal {

void StoreArg(State* state, int arg, bool ref) {
  // DO NOT USE ANY C++ STACK BEFORE THE UNSAFE CALLS.
  CHECK_EQ(GetType(state, 1), LuaType::UserData);
  // this.__yuerefs ?= {}
  UnsafeGet(state, 1, "__yuerefs");
  if (GetType(state, -1) != LuaType::Table) {
    NewTable(state, 0, 1);
    UnsafeSet(state, 1, "__yuerefs", ValueOnStack(state, -1));
  }
  // this.__yuerefs[arg] = ref ? 1 : nil
  if (ref)
    RawSet(state, -1, ValueOnStack(state, arg), 1);
  else
    RawSet(state, -1, ValueOnStack(state, arg), nullptr);
  // Cleanup stack.
  PopAndIgnore(state, 1);
}

}  // namespace internal

}  // namespace lua
