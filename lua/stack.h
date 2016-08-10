// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
//
// Helper functions to manipulate lua's stack.

#ifndef LUA_STACK_H_
#define LUA_STACK_H_

#include "lua/types.h"

namespace lua {

// Needed by the arbitrary length version of Push.
inline bool Push(State* state) {
  return true;
}

// Enable push arbitrary args at the same time.
template<typename ArgType, typename... ArgTypes>
inline bool Push(State* state, ArgType arg, ArgTypes... args) {
  return Type<ArgType>::Push(state, arg) && Push(state, args...);
}

// Helpers for pushing strings.
PRINTF_FORMAT(2, 3)
inline bool PushFormatedString(State* state,
                               _Printf_format_string_ const char* format,
                               ...)  {
  va_list ap;
  va_start(ap, format);
  lua_pushvfstring(state, format, ap);
  va_end(ap);
  return true;  // ignore memory errors.
}

// Needed by the arbitrary length version of toxxx.
inline bool To(State* state, int index) {
  return true;
}

// Enable getting arbitrary args at the same time.
template<typename ArgType, typename... ArgTypes>
inline bool To(State* state, int index, ArgType* arg, ArgTypes... args) {
  return Type<ArgType>::To(state, index, arg) && To(state, index + 1, args...);
}

// Thin wrapper for lua_pop.
inline void Pop(State* state, size_t n) {
  lua_pop(state, n);
}

// Get the values and pop them from statck.
template<typename T>
inline bool Pop(State* state, T* result) {
  if (To(state, -1, result)) {
    Pop(state, 1);
    return true;
  } else {
    return false;
  }
}

// Enable poping arbitrary args at the same time.
template<typename... ArgTypes>
inline bool Pop(State* state, ArgTypes... args) {
  if (To(state, -static_cast<int>(sizeof...(args)), args...)) {
    Pop(state, sizeof...(args));
    return true;
  } else {
    return false;
  }
}

// Thin wrappers of settop/gettop.
inline void SetTop(State* state, int index) {
  lua_settop(state, index);
}

inline int GetTop(State* state) {
  return lua_gettop(state);
}

}  // namespace lua

#endif  // LUA_STACK_H_
