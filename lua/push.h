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

// Enable push arbitrary args at the same time.
template<typename ArgType, typename... ArgTypes>
inline bool Push(State* state, ArgType arg, ArgTypes... args) {
  return Push(state, arg) && Push(state, args...);
}

// Needed by the arbitrary length version of Push.
inline bool Push(State* state) {
  return true;
}

// Specialized push for each type.
inline bool Push(State* state, lua_Integer number) {
  lua_pushinteger(state, number);
  return true;
}

inline bool Push(State* state, base::StringPiece str) {
  lua_pushlstring(state, str.data(), str.length());
  return true;  // ignore memory errors.
}

}  // namespace lua

#endif  // LUA_PUSH_H_
