// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.
//
// The lua APIs that do not belong to any other category.

#ifndef LUA_LUA_H_
#define LUA_LUA_H_

#include "lua/state.h"

namespace lua {

inline void SetTop(State* state, int index) {
  lua_settop(state, index);
}

inline int GetTop(State* state) {
  return lua_gettop(state);
}

}  // namespace lua

#endif  // LUA_LUA_H_
