// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_PCALL_H_
#define LUA_PCALL_H_

#include "lua/stack.h"

namespace lua {

// Safely call the function on top of stack.
// When error happens, this API returns false and leaves the error on stack.
template<typename ReturnType, typename... ArgTypes>
inline bool PCall(State* state, ReturnType* result, const ArgTypes&... args) {
  Push(state, args...);

  if (lua_pcall(state, sizeof...(ArgTypes), Values<ReturnType>::count,
                0 /* no message handler */) != LUA_OK)
    return false;

  return Pop(state, result);
}

// Passing nullptr means there is no result expected.
template<typename... ArgTypes>
inline bool PCall(State* state, nullptr_t, const ArgTypes&... args) {
  Push(state, args...);
  return lua_pcall(state, sizeof...(ArgTypes), 0, 0) == LUA_OK;
}

}  // namespace lua

#endif  // LUA_PCALL_H_
