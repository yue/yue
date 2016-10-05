// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "yue/api_util.h"

#include <iostream>
#include <sstream>

#include "base/strings/string_number_conversions.h"

namespace {

// Options for inspecting string.
struct InspectOptions {
  int indention = 2;
  int break_length = 60;
};

// Forward declaration.
std::string ConvertToString(lua::State* state, int index, int indent = 0,
                            const InspectOptions& options = InspectOptions());

// Determine whether the table at |index| is a table.
// TODO(zcbenz): Use private lua API to determine this.
bool IsArray(lua::State* state, int index) {
  lua::StackAutoReset reset(state);
  int last = 0;
  lua::PushNil(state);
  while (lua_next(state, index)) {
    if (lua::GetType(state, -2) != lua::LuaType::Number)
      return false;
    int current;
    if (!lua::To(state, -2, &current) || current != last + 1)
      return false;
    last = current;
    lua::PopAndIgnore(state, 1);
  }
  return true;
}

std::string TableToString(lua::State* state, int index, int indent,
                          const InspectOptions& options) {
  lua::StackAutoReset reset(state);
  bool is_array = IsArray(state, index);
  std::stringstream str;
  str << std::string(indent, ' ');
  str << "{ ";
  lua::PushNil(state);
  bool first = true;
  while (lua_next(state, index)) {
    if (first)
      first = false;
    else
      str << ", ";
    if (!is_array)
      str << ConvertToString(state, -2) << "=";
    str << ConvertToString(state, -1);
    lua::PopAndIgnore(state, 1);
  }
  str << " }";
  return str.str();
}

std::string ConvertToString(lua::State* state, int index, int indent,
                            const InspectOptions& options) {
  std::string str;
  lua::LuaType type = lua::GetType(state, index);
  switch (type) {
    case lua::LuaType::None:
      str = "<none>";
      break;
    case lua::LuaType::Nil:
      str = "nil";
      break;
    case lua::LuaType::Number:
      if (lua_isinteger(state, index))
        str = base::IntToString(lua_tointeger(state, index));
      else
        str = base::DoubleToString(lua_tonumber(state, index));
      break;
    case lua::LuaType::String:
      str = lua_tostring(state, index);
      break;
    case lua::LuaType::Boolean:
      str = lua_toboolean(state, index) ? "true" : "false";
      break;
    case lua::LuaType::Function:
      str = "<function>";
      break;
    case lua::LuaType::UserData:
      str = "<userdata>";
      break;
    case lua::LuaType::Thread:
      str = "<thread>";
      break;
    case lua::LuaType::LightUserData:
      str = "<lightuserdata>";
      break;
    case lua::LuaType::Table:
      return TableToString(state, index, indent, options);
  }
  return std::string(indent, ' ') + str;
}

int Inspect(lua::State* state) {
  lua::Push(state, ConvertToString(state, -1));
  return 1;
}

int Print(lua::State* state) {
  int argc = lua::GetTop(state);
  for (int i = 1; i <= argc; ++i) {
    std::cout << ConvertToString(state, i);
    if (i != argc)
      std::cout << " ";
  }
  std::cout << std::endl;
  return 0;
}

}  // namespace

extern "C" int luaopen_yue_util(lua::State* state) {
  lua::PushNewTable(state);
  lua::RawSet(state, -1, "inspect", lua::CFunction(&Inspect));
  lua::RawSet(state, -1, "print", lua::CFunction(&Print));
  return 1;
}
