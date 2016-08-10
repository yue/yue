// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
//
// Helper functions for pushing values to stack.

#ifndef LUA_PUSH_H_
#define LUA_PUSH_H_

#include <string>

#include "base/strings/string_piece.h"
#include "lua/state.h"

namespace lua {

// Specialized push for each type.
inline bool Push(State* state, int number) {
  lua_pushinteger(state, number);
  return true;
}

inline bool Push(State* state, nullptr_t) {
  lua_pushnil(state);
  return true;
}

inline bool Push(State* state, bool b) {
  lua_pushboolean(state, b);
  return true;
}

inline bool Push(State* state, base::StringPiece str) {
  lua_pushlstring(state, str.data(), str.length());
  return true;  // ignore memory errors.
}

inline bool Push(State* state, const std::string& str) {
  lua_pushlstring(state, str.data(), str.length());
  return true;  // ignore memory errors.
}

inline bool Push(State* state, const char* str) {
  lua_pushstring(state, str);
  return true;  // ignore memory errors.
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

// Needed by the arbitrary length version of Push.
inline bool Push(State* state) {
  return true;
}

// Enable push arbitrary args at the same time.
template<typename ArgType, typename... ArgTypes>
inline bool Push(State* state, ArgType arg, ArgTypes... args) {
  return Push(state, arg) && Push(state, args...);
}

}  // namespace lua

#endif  // LUA_PUSH_H_
