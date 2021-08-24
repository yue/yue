// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_PCALL_H_
#define LUA_PCALL_H_

#include <utility>

#include "lua/stack.h"

namespace lua {

// Safely call the function on top of stack.
// When error happens, this API returns false and leaves the error on stack.
template<typename ReturnType, typename... ArgTypes>
inline bool PCall(State* state, ReturnType* result, ArgTypes&&... args) {
  Push(state, std::forward<ArgTypes>(args)...);

  if (lua_pcall(state, sizeof...(ArgTypes), Values<ReturnType>::count,
                0 /* no message handler */) != LUA_OK)
    return false;

  // When failed to convert return value.
  if (!Pop(state, result)) {
    if (GetType(state, -1) == LuaType::Nil) {
      // Use the type's default value for nil.
      *result = std::move(ReturnType());
      return true;
    } else {
      // Otherwise throw error.
      const char* name = GetTypeName(state, -1);
      PopAndIgnore(state, 1);
      PushFormatedString(state, "error converting return value from %s to %s",
                         name, Type<ReturnType>::name);
      return false;
    }
  }

  return true;
}

// Passing nullptr means there is no result expected.
template<typename... ArgTypes>
inline bool PCall(State* state, std::nullptr_t, ArgTypes&&... args) {
  Push(state, std::forward<ArgTypes>(args)...);
  return lua_pcall(state, sizeof...(ArgTypes), 0, 0) == LUA_OK;
}

}  // namespace lua

#endif  // LUA_PCALL_H_
