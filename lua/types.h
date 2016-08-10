// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
//
// Defines how to convert types between lua and C++.

#ifndef LUA_TYPES_H_
#define LUA_TYPES_H_

#include <string>
#include <tuple>

#include "base/strings/string_piece.h"
#include "lua/state.h"

namespace lua {

// Possible lua types.
enum class LuaType {
  None          = LUA_TNONE,
  Nil           = LUA_TNIL,
  Number        = LUA_TNUMBER,
  Boolean       = LUA_TBOOLEAN,
  String        = LUA_TSTRING,
  Table         = LUA_TTABLE,
  Function      = LUA_TFUNCTION,
  UserData      = LUA_TUSERDATA,
  Thread        = LUA_TTHREAD,
  LightUserData = LUA_TLIGHTUSERDATA,
};

// Get how many lua values the type represents.
template<typename T>
struct Values {
  static const int count = 1;
};

template<>
struct Values<void> {
  static const int count = 0;
};

template<typename... ArgTypes>
struct Values<std::tuple<ArgTypes...>> {
  static const int count = sizeof...(ArgTypes);
};

// Defines how C++ types and lua types re converted.
template<typename T, typename Enable = void>
struct Type {};

template<>
struct Type<int> {
  static constexpr const char* name = "integer";
  static inline void Push(State* state, int number) {
    lua_pushinteger(state, number);
  }
  static inline bool To(State* state, int index, int* out) {
    int success = 0;
    int ret = lua_tointegerx(state, index, &success);
    if (success)
      *out = ret;
    return success;
  }
};

template<>
struct Type<double> {
  static constexpr const char* name = "number";
  static inline void Push(State* state, double number) {
    lua_pushnumber(state, number);
  }
  static inline bool To(State* state, int index, double* out) {
    int success = 0;
    int ret = lua_tonumberx(state, index, &success);
    if (success)
      *out = ret;
    return success;
  }
};

template<>
struct Type<bool> {
  static constexpr const char* name = "boolean";
  static inline void Push(State* state, bool b) {
    lua_pushboolean(state, b);
  }
  static inline bool To(State* state, int index, bool* out) {
    if (!lua_isboolean(state, index))
      return false;
    *out = lua_toboolean(state, index);
    return true;
  }
};

template<>
struct Type<nullptr_t> {
  static constexpr const char* name = "nil";
  static inline void Push(State* state, nullptr_t) {
    lua_pushnil(state);
  }
};

template<>
struct Type<base::StringPiece> {
  static constexpr const char* name = "string";
  static inline void Push(State* state, base::StringPiece str) {
    lua_pushlstring(state, str.data(), str.length());
  }
  static inline bool To(State* state, int index, base::StringPiece* out) {
    const char* str = lua_tostring(state, index);
    if (!str)
      return false;
    *out = str;
    return true;  // ignore memory errors.
  }
};

template<>
struct Type<std::string> {
  static constexpr const char* name = "string";
  static inline void Push(State* state, const std::string& str) {
    lua_pushlstring(state, str.data(), str.length());
  }
  static inline bool To(State* state, int index, std::string* out) {
    const char* str = lua_tostring(state, index);
    if (!str)
      return false;
    *out = str;
    return true;  // ignore memory errors.
  }
};

template<>
struct Type<const char*> {
  static inline void Push(State* state, const char* str) {
    lua_pushstring(state, str);
  }
};

}  // namespace lua

#endif  // LUA_TYPES_H_
