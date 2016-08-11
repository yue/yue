// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_HANDLE_H_
#define LUA_HANDLE_H_

#include <memory>

#include "lua/state.h"

namespace lua {

// Puts the value of index to registry and holds a reference to it.
class Handle {
 public:
  static std::unique_ptr<Handle> New(State* state, int index) {
    lua_pushvalue(state, index);
    return std::unique_ptr<Handle>(new Handle(state));
  }

  ~Handle() {
    luaL_unref(state_, LUA_REGISTRYINDEX, ref_);
  }

  // Puts the value back to stack.
  void Push() const { lua_rawgeti(state_, LUA_REGISTRYINDEX, ref_); }

 protected:
  explicit Handle(State* state)
      : state_(state), ref_(luaL_ref(state, LUA_REGISTRYINDEX)) {}

 private:
  State* state_;
  int ref_;

  DISALLOW_COPY_AND_ASSIGN(Handle);
};

}  // namespace lua

#endif  // LUA_HANDLE_H_
