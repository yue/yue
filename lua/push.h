// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.
//
// Helper functions for pushing values to stack.

#ifndef LUA_PUSH_H_
#define LUA_PUSH_H_

#include "base/strings/string_piece.h"
#include "lua/state.h"

namespace lua {

inline void Push(State* state, lua_Integer number) {
  lua_pushinteger(state, number);
}

inline void Push(State* state, base::StringPiece str) {
  lua_pushlstring(state, str.data(), str.length());
}

}  // namespace lua

#endif  // LUA_PUSH_H_
