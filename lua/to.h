// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.
//
// Helper functions for gettings values on stack.

#ifndef LUA_TO_H_
#define LUA_TO_H_

#include <string>

#include "base/strings/string_piece.h"
#include "lua/state.h"

namespace lua {

// Specialized toxxx for each type.
inline bool To(State* state, int index, int* out) {
  int success = 0;
  int ret = lua_tointegerx(state, index, &success);
  if (success)
    *out = ret;
  return success;
}

inline bool To(State* state, int index, base::StringPiece* out) {
  const char* str = lua_tostring(state, index);
  if (!str)
    return false;
  *out = str;
  return true;
}

inline bool To(State* state, int index, std::string* out) {
  const char* str = lua_tostring(state, index);
  if (!str)
    return false;
  *out = str;
  return true;  // ignore memory errors.
}

// Needed by the arbitrary length version of toxxx.
inline bool To(State* state, int index) {
  return true;
}

// Enable getting arbitrary args at the same time.
template<typename ArgType, typename... ArgTypes>
inline bool To(State* state, int index, ArgType* arg, ArgTypes... args) {
  return To(state, index, arg) && To(state, index + 1, args...);
}

}  // namespace lua

#endif  // LUA_TO_H_
