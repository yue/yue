// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.
//
// Extending the APIs for base::Callback.

#ifndef LUA_CALLBACK_H_
#define LUA_CALLBACK_H_

#include "lua/callback_internal.h"
#include "lua/pop.h"
#include "lua/push.h"

namespace lua {

// The push operation for callback.
template<typename Sig>
inline bool Push(State* state, const base::Callback<Sig>& callback) {
  return false;
}

// Safely call the function under lua environment,
// When error happens, this API returns false and leaves the error on stack.
template<typename ReturnType, typename... ArgTypes>
inline bool PCall(State* state,
                  const base::Callback<ReturnType(ArgTypes...)>& callback,
                  ReturnType* result, ArgTypes... args) {
  if (!internal::PushCFunction(state, callback) || !Push(state, args...))
    return false;

  if (lua_pcall(state, sizeof...(ArgTypes),
                internal::ReturnValues<ReturnType>::count,
                0 /* no message handler */) != LUA_OK)
    return false;

  return Pop(state, result);
}

// The PCall version without return value.
template<typename... ArgTypes>
inline bool PCall(State* state,
                  const base::Callback<void(ArgTypes...)>& callback,
                  ArgTypes... args) {
  if (!internal::PushCFunction(state, callback) || !Push(state, args...))
    return false;

  return lua_pcall(state, sizeof...(ArgTypes), 0, 0) == LUA_OK;
}

}  // namespace lua

#endif  // LUA_CALLBACK_H_
