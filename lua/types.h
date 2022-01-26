// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
//
// Defines how to convert types between lua and C++.

#ifndef LUA_TYPES_H_
#define LUA_TYPES_H_

#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "lua/stack_auto_reset.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

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
struct Type<lua_Integer> {
  static constexpr const char* name = "integer";
  static inline void Push(State* state, lua_Integer number) {
    lua_pushinteger(state, number);
  }
  static inline bool To(State* state, int index, lua_Integer* out) {
    int success = 0;
    int ret = lua_tointegerx(state, index, &success);
    if (success)
      *out = ret;
    return success != 0;
  }
};

#if defined(ARCH_CPU_64_BITS) || LUA_VERSION_NUM > 501
template<>
struct Type<int> {
  static constexpr const char* name = "integer";
  static inline void Push(State* state, int number) {
    lua_pushinteger(state, number);
  }
  static inline bool To(State* state, int index, int* out) {
    lua_Integer integer;
    if (!Type<lua_Integer>::To(state, index, &integer))
      return false;
    *out = static_cast<int>(integer);
    return true;
  }
};
#endif

template<>
struct Type<uint32_t> {
  static constexpr const char* name = "integer";
  static inline void Push(State* state, uint32_t number) {
    lua_pushinteger(state, number);
  }
  static inline bool To(State* state, int index, uint32_t* out) {
    lua_Integer integer;
    if (!Type<lua_Integer>::To(state, index, &integer))
      return false;
    *out = static_cast<uint32_t>(integer);
    return true;
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
    float ret = lua_tonumberx(state, index, &success);
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
  static inline void Push(State* state, const void* ptr) {
    lua_pushlightuserdata(state, const_cast<void*>(ptr));
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
struct Type<std::wstring> {
  static constexpr const char* name = "string";
  static inline void Push(State* state, const std::wstring& str) {
    Type<std::string>::Push(state, base::WideToUTF8(str));
  }
  static inline bool To(State* state, int index, std::wstring* out) {
    const char* str = lua_tostring(state, index);
    if (!str)
      return false;
    *out = base::UTF8ToWide(str);
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

template<typename T>
struct Type<absl::optional<T>> {
  static constexpr const char* name = Type<T>::name;
  static inline void Push(State* state, const absl::optional<T>& value) {
    if (value)
      Type<T>::Push(state, *value);
    else
      lua_pushnil(state);
  }
  static inline bool To(State* state, int index, absl::optional<T>* out) {
    if (GetType(state, index) == LuaType::Nil) {
      out->reset();
      return true;
    }
    T copy;
    if (!Type<T>::To(state, index, &copy))
      return false;
    out->emplace(std::move(copy));
    return true;
  }
};

// Provide type description for tuple type.
template<typename... ArgTypes>
struct Type<std::tuple<ArgTypes...>> {
  static constexpr const char* name = "tuple<>";
};

template<typename T>
struct Type<std::vector<T>> {
  static constexpr const char* name = "table";
  static inline void Push(State* state, const std::vector<T>& vec) {
    int size = static_cast<int>(vec.size());  // lua does not like size_t
    lua_createtable(state, size, 0);
    for (int i = 0; i < size; ++i) {
      Type<T>::Push(state, vec[i]);
      lua_rawseti(state, -2, i + 1);
    }
  }
  static inline bool To(State* state, int index, std::vector<T>* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    StackAutoReset reset(state);
    lua_pushnil(state);
    while (lua_next(state, index) != 0) {
      if (GetType(state, -2) != LuaType::Number ||  // check array type
          lua_tointeger(state, -2) != static_cast<int>(out->size() + 1))
        return false;
      T value;
      if (!Type<T>::To(state, -1, &value))
        return false;
      lua_pop(state, 1);
      out->push_back(std::move(value));
    }
    return true;
  }
};

template<typename T>
struct Type<std::set<T>> {
  static constexpr const char* name = "table";
  static inline void Push(State* state, const std::set<T>& vec) {
    int size = static_cast<int>(vec.size());  // lua does not like size_t
    lua_createtable(state, size, 0);
    int i = 0;
    for (const T& element : vec) {
      Type<T>::Push(state, element);
      lua_rawseti(state, -2, ++i);
    }
  }
  static inline bool To(State* state, int index, std::set<T>* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    StackAutoReset reset(state);
    lua_pushnil(state);
    while (lua_next(state, index) != 0) {
      if (GetType(state, -2) != LuaType::Number ||  // check array type
          lua_tointeger(state, -2) != static_cast<int>(out->size() + 1))
        return false;
      T value;
      if (!Type<T>::To(state, -1, &value))
        return false;
      lua_pop(state, 1);
      out->insert(std::move(value));
    }
    return true;
  }
};

template<typename K, typename V>
struct Type<std::map<K, V>> {
  static constexpr const char* name = "table";
  static inline void Push(State* state, const std::map<K, V>& dict) {
    lua_createtable(state, 0, dict.size());
    for (const auto& it : dict) {
      Type<K>::Push(state, it.first);
      Type<V>::Push(state, it.second);
      lua_rawset(state, -3);
    }
  }
  static inline bool To(State* state, int index, std::map<K, V>* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    StackAutoReset reset(state);
    lua_pushnil(state);
    while (lua_next(state, index) != 0) {
      K key;
      V value;
      if (!Type<K>::To(state, -2, &key) || !Type<V>::To(state, -1, &value))
        return false;
      lua_pop(state, 1);
      (*out)[key] = std::move(value);
    }
    return true;
  }
};

}  // namespace lua

#endif  // LUA_TYPES_H_
