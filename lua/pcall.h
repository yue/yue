// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
//
// Extending the APIs for base::Callback.

#ifndef LUA_PCALL_H_
#define LUA_PCALL_H_

#include "lua/pcall_internal.h"

namespace lua {

// The push operation for callback.
template<typename Sig>
inline bool Push(State* state, const base::Callback<Sig>& callback) {
  return internal::PushCFunction(state, callback);
}

// Safely call the function on top of stack.
// When error happens, this API returns false and leaves the error on stack.
template<typename ReturnType, typename... ArgTypes>
inline bool PCall(State* state, ReturnType* result, ArgTypes... args) {
  if (!Push(state, args...))
    return false;

  if (lua_pcall(state, sizeof...(ArgTypes), Values<ReturnType>::count,
                0 /* no message handler */) != LUA_OK)
    return false;

  return Pop(state, result);
}

// Passing nullptr means there is no result expected.
template<typename... ArgTypes>
inline bool PCall(State* state, nullptr_t, ArgTypes... args) {
  return Push(state, args...) &&
         lua_pcall(state, sizeof...(ArgTypes), 0, 0) == LUA_OK;
}

}  // namespace lua

#endif  // LUA_PCALL_H_
