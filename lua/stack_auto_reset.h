// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_STACK_AUTO_RESET_H_
#define LUA_STACK_AUTO_RESET_H_

#include "lua/state.h"

namespace lua {

// Thin wrappers of settop.
inline void SetTop(State* state, int index) {
  lua_settop(state, index);
}

inline int GetTop(State* state) {
  return lua_gettop(state);
}

// Automatically clear the values on stack.
class StackAutoReset {
 public:
  explicit StackAutoReset(State* state) : state_(state), top_(GetTop(state)) {}
  ~StackAutoReset() { SetTop(state_, top_); }

  StackAutoReset& operator=(const StackAutoReset&) = delete;
  StackAutoReset(const StackAutoReset&) = delete;

  int top() const { return top_; }

 private:
  State* state_;
  int top_;
};

}  // namespace lua

#endif  // LUA_STACK_AUTO_RESET_H_
