// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
//
// Provides the names of C++ types, used by PCall to inform the information of
// the arguments of functions.

#ifndef LUA_TYPE_NAME_H_
#define LUA_TYPE_NAME_H_

#include <string>
#include <tuple>

#include "base/strings/string_piece.h"

namespace lua {

template<typename T>
struct TypeName {
  static constexpr const char* value = "userdata";
};

template<>
struct TypeName<int> {
  static constexpr const char* value = "integer";
};

template<>
struct TypeName<nullptr_t> {
  static constexpr const char* value = "nil";
};

template<>
struct TypeName<bool> {
  static constexpr const char* value = "boolean";
};

template<>
struct TypeName<std::string> {
  static constexpr const char* value = "string";
};

}  // namespace lua

#endif  // LUA_TYPE_NAME_H_
