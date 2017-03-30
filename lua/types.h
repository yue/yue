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
#include "base/strings/utf_string_conversions.h"
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

// Thin wrapper of lua_type.
inline LuaType GetType(State* state, int index) {
  return static_cast<LuaType>(lua_type(state, index));
}

inline const char* GetTypeName(State* state, int index) {
  return lua_typename(state, lua_type(state, index));
}

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
    return success != 0;
  }
};

template<>
struct Type<uint32_t> {
  static constexpr const char* name = "integer";
  static inline void Push(State* state, uint32_t number) {
    lua_pushinteger(state, number);
  }
  static inline bool To(State* state, int index, uint32_t* out) {
    int success = 0;
    int ret = lua_tointegerx(state, index, &success);
    if (success)
      *out = ret;
    return success != 0;
  }
};

template<>
struct Type<float> {
  static constexpr const char* name = "number";
  static inline void Push(State* state, float number) {
    lua_pushnumber(state, number);
  }
  static inline bool To(State* state, int index, float* out) {
    int success = 0;
    int ret = lua_tonumberx(state, index, &success);
    if (success)
      *out = ret;
    return success != 0;
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
    return success != 0;
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
    *out = lua_toboolean(state, index) != 0;
    return true;
  }
};

template<>
struct Type<std::nullptr_t> {
  static constexpr const char* name = "nil";
  static inline void Push(State* state, std::nullptr_t) {
    lua_pushnil(state);
  }
};

template<>
struct Type<void*> {
  static constexpr const char* name = "lightuserdata";
  static inline void Push(State* state, void* ptr) {
    lua_pushlightuserdata(state, ptr);
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
struct Type<base::string16> {
  static constexpr const char* name = "string";
  static inline void Push(State* state, const base::string16& str) {
    Type<std::string>::Push(state, base::UTF16ToUTF8(str));
  }
  static inline bool To(State* state, int index, base::string16* out) {
    const char* str = lua_tostring(state, index);
    if (!str)
      return false;
    *out = base::UTF8ToUTF16(str);
    return true;  // ignore memory errors.
  }
};

template<>
struct Type<const char*> {
  static constexpr const char* name = "string";
  static inline void Push(State* state, const char* str) {
    lua_pushstring(state, str);
  }
  static inline bool To(State* state, int index, const char** out) {
    const char* str = lua_tostring(state, index);
    if (!str)
      return false;
    *out = str;
    return true;  // ignore memory errors.
  }
};

template<size_t n>
struct Type<const char[n]> {
  static constexpr const char* name = "string";
  static inline void Push(State* state, const char* str) {
    lua_pushlstring(state, str, n - 1);
  }
};

// Provide type description for tuple type.
template<typename... ArgTypes>
struct Type<std::tuple<ArgTypes...>> {
  static constexpr const char* name = "tuple<>";
};

}  // namespace lua

#endif  // LUA_TYPES_H_
