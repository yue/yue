// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_POP_H_
#define LUA_POP_H_

#include "lua/to.h"

namespace lua {

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

}  // namespace lua

#endif  // LUA_POP_H_
