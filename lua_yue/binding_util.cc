// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua_yue/binding_util.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"

namespace {

// Context for inspecting string.
struct InspectContext {
  int indent = 2;
  int break_length = 60;
};

// Forward declaration.
std::string ConvertToString(lua::State* state, int index, InspectContext* ctx);

// Determine whether the table at |index| is a table.
// TODO(zcbenz): Use private lua API to determine this.
bool IsArray(lua::State* state, int index) {
  lua::StackAutoReset reset(state);
  index = lua::AbsIndex(state, index);
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

// Join the results and break line when needed.
std::string ReduceToSingleString(const std::vector<std::string>& output,
                                 InspectContext* ctx) {
  if (output.empty())
    return "{}";
  int length = std::accumulate(
      output.begin(), output.end(), 0,
      [](int sum, const std::string str) -> int {
        return sum + static_cast<int>(str.length()) + 1;
      });
  if (length > ctx->break_length)
    return "{ " +
           base::JoinString(output, ",\n" + std::string(ctx->indent, ' ')) +
           " }";
  return "{ " + base::JoinString(output, ", ") + " }";
}

std::string TableToString(lua::State* state, int index, InspectContext* ctx) {
  lua::StackAutoReset reset(state);
  index = lua::AbsIndex(state, index);
  // Whether the table only contains sequence data.
  bool is_array = IsArray(state, index);
  // Elements are pushed into an vector and joined later.
  std::vector<std::string> output;
  // Increase indention for child tables.
  InspectContext child_ctx = *ctx;
  child_ctx.indent += 2;
  child_ctx.break_length -= 2;
  // Start table iteration.
  lua::PushNil(state);
  while (lua_next(state, index)) {
    std::string value = ConvertToString(state, -1, &child_ctx);
    if (is_array)
      output.push_back(value);
    else
      output.push_back(ConvertToString(state, -2, &child_ctx) + " = " + value);
    lua::PopAndIgnore(state, 1);
  }
  if (!is_array)  // dictionary should be ordered.
    std::sort(output.begin(), output.end());
  return ReduceToSingleString(output, ctx);
}

std::string ConvertToString(lua::State* state, int index, InspectContext* ctx) {
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
        str = base::NumberToString(lua_tointeger(state, index));
      else
        str = base::NumberToString(lua_tonumber(state, index));
      break;
    case lua::LuaType::String:
      str = std::string("\"") + lua_tostring(state, index) + "\"";
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
      return TableToString(state, index, ctx);
  }
  return str;
}

int Inspect(lua::State* state) {
  InspectContext ctx;
  lua::Push(state, ConvertToString(state, -1, &ctx));
  return 1;
}

int Print(lua::State* state) {
  int argc = lua::GetTop(state);
  for (int i = 1; i <= argc; ++i) {
    InspectContext ctx;
    std::cout << ConvertToString(state, i, &ctx);
    if (i != argc)
      std::cout << " ";
  }
  std::cout << std::endl;
  return 0;
}

}  // namespace

extern "C" int luaopen_yue_util(lua::State* state) {
  lua::NewTable(state);
  lua::RawSet(state, -1, "inspect", lua::CFunction(&Inspect));
  lua::RawSet(state, -1, "print", lua::CFunction(&Print));
  return 1;
}
