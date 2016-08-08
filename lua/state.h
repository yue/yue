// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef LUA_STATE_H_
#define LUA_STATE_H_

#include "base/macros.h"
#include "third_party/lua/src/lua.hpp"

namespace lua {

// Avoid using lua_State directly.
using State = lua_State;

// Manages the created lua_State.
class MangedState {
 public:
  MangedState() : state_(luaL_newstate()) {
  }

  ~MangedState() {
    lua_close(state_);
  }

  operator State*() { return state_; }

 private:
  State* state_;

  DISALLOW_COPY_AND_ASSIGN(MangedState);
};

}  // namespace lua

#endif  // LUA_STATE_H_
