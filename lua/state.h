// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_STATE_H_
#define LUA_STATE_H_

#include "base/macros.h"

extern "C" {
#include "third_party/lua-compat-5.3/c-api/compat-5.3.h"
}

namespace lua {

// Avoid using lua_State directly.
using State = lua_State;

// Manages the created lua_State.
class ManagedState {
 public:
  ManagedState() : state_(luaL_newstate()) {
  }

  ~ManagedState() {
    lua_close(state_);
  }

  operator State*() { return state_; }

 private:
  State* state_;

  DISALLOW_COPY_AND_ASSIGN(ManagedState);
};

}  // namespace lua

#endif  // LUA_STATE_H_
