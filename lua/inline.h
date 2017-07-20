// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_INLINE_H_
#define LUA_INLINE_H_

#include "lua/state.h"

extern "C" {
#include "third_party/lua/src/lstate.h"
}

namespace lua {

// Inline versions of common lua operations.
// This requires access to private headers of lua and does not work together
// with LuaJIT.

inline int AbsIndex(State* L, int index) {
  return (index > 0 || index <= LUA_REGISTRYINDEX) ?
      index : static_cast<int>(L->top - L->ci->func) + index;
}

inline int GetTop(State* L) {
  return static_cast<int>(L->top - (L->ci->func + 1));
}

}  // namespace lua

#endif  // LUA_INLINE_H_
