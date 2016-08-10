// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_CALL_CONTEXT_H_
#define LUA_CALL_CONTEXT_H_

#include "lua/state.h"

namespace lua {

enum PCallFunctionFlags {
  HolderIsFirstArgument = 1 << 0,
};

// A class used by PCall to provide information of current call.
struct CallContext {
  explicit CallContext(State* state, int create_flags)
      : state(state), create_flags(create_flags) {}

  // The lua state.
  State* state;

  // Whether there is error on stack.
  bool has_error = false;

  // The flags passed when creating the call.
  const int create_flags = 0;

  // The index (1-based) of the arg that failed to convert.
  int invalid_arg = 0;

  // The name of the invalid arg.
  const char* invalid_arg_name = nullptr;
};

}  // namespace lua

#endif  // LUA_CALL_CONTEXT_H_
