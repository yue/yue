// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_CALL_CONTEXT_H_
#define LUA_CALL_CONTEXT_H_

#include "lua/state.h"

namespace lua {

// A class used by PCall to provide information of current call.
struct CallContext {
  explicit CallContext(State* state) : state(state) {}

  // The lua state.
  State* state;

  // How many return values we have.
  int return_values_count = 0;

  // Whether there is error on stack.
  bool has_error = false;

  // The index (1-based) of current arg.
  int current_arg = 1;

  // The index (1-based) of the arg that failed to convert.
  int invalid_arg = 0;

  // The name of the invalid arg.
  const char* invalid_arg_name = nullptr;
};

}  // namespace lua

#endif  // LUA_CALL_CONTEXT_H_
