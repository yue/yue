// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/ref_method.h"

namespace lua {

namespace internal {

void StoreArg(State* state, int arg, RefType ref_type, const char* ref_key) {
  // DO NOT USE ANY C++ STACK BEFORE THE UNSAFE CALLS.
  CHECK_EQ(GetType(state, 1), LuaType::UserData);
  // this.__yuerefs ?= {}
  PushRefsTable(state, "__yuerefs", 1);
  // if (reftype == "ref")
  //   this.__yuerefs[arg] = true
  // else if (reftype == "deref")
  //   this.__yuerefs[arg] = nil
  // else if (reftype == "reset")
  //   this.__yuerefs[key] = arg
  switch (ref_type) {
    case RefType::Ref:
      RawSet(state, -1, ValueOnStack(state, arg), 1);
      break;
    case RefType::Deref:
      RawSet(state, -1, ValueOnStack(state, arg), nullptr);
      break;
    case RefType::Reset:
      RawSet(state, -1, ref_key, ValueOnStack(state, arg));
      break;
  }
  // Cleanup stack.
  PopAndIgnore(state, 1);
}

}  // namespace internal

}  // namespace lua
