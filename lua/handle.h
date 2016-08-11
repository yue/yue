// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_HANDLE_H_
#define LUA_HANDLE_H_

#include "lua/state.h"

namespace lua {

// Puts the value on top of stack to registry and holds a reference to it.
class Handle {
 public:
  explicit Handle(State* state)
      : state_(state), ref_(luaL_ref(state, LUA_REGISTRYINDEX)) {}
  ~Handle() { luaL_unref(state_, LUA_REGISTRYINDEX, ref_); }

  // Puts the value back to stack.
  void Get() { lua_rawgeti(state_, LUA_REGISTRYINDEX, ref_); }

 private:
  State* state_;
  int ref_;

  DISALLOW_COPY_AND_ASSIGN(Handle);
};

}  // namespace lua

#endif  // LUA_HANDLE_H_
