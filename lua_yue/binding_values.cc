// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua_yue/binding_values.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/memory/ptr_util.h"

namespace lua {

namespace {

// Check if a table is array.
bool IsTableArray(State* state, int index, size_t* size) {
  StackAutoReset reset(state);
  *size = 0;
  lua_pushnil(state);
  while (lua_next(state, index) != 0) {
    if (GetType(state, -2) != LuaType::Number ||  // check array type
        lua_tointeger(state, -2) != static_cast<int>(*size + 1))
      return false;
    (*size)++;
    lua_pop(state, 1);
  }
  return true;
}

}  // namespace

// static
void Type<base::Value>::Push(State* state, const base::Value& value) {
  switch (value.type()) {
    case base::Value::Type::NONE:
      lua::PushNil(state);
      return;
    case base::Value::Type::BOOLEAN:
      lua::Push(state, value.GetBool());
      return;
    case base::Value::Type::INTEGER:
      lua::Push(state, value.GetInt());
      return;
    case base::Value::Type::DOUBLE:
      lua::Push(state, value.GetDouble());
      return;
    case base::Value::Type::STRING:
      lua::Push(state, value.GetString());
      return;
    case base::Value::Type::BINARY:
      lua_pushlstring(state,
                      reinterpret_cast<const char*>(value.GetBlob().data()),
                      value.GetBlob().size());
      return;
    case base::Value::Type::DICTIONARY: {
      NewTable(state, 0, static_cast<int>(value.DictSize()));
      for (const auto it : value.DictItems()) {
        lua::Push(state, it.first);
        Type<base::Value>::Push(state, it.second);
        lua_rawset(state, -3);
      }
      return;
    }
    case base::Value::Type::LIST: {
      NewTable(state, static_cast<int>(value.GetList().size()), 0);
      for (size_t i = 0; i < value.GetList().size(); ++i) {
        lua::Push(state, static_cast<int>(i + 1));
        Type<base::Value>::Push(state, value.GetList()[i]);
        lua_rawset(state, -3);
      }
      return;
    }
  }
}

// static
bool Type<base::Value>::To(State* state, int index, base::Value* out) {
  index = AbsIndex(state, index);
  switch (GetType(state, index)) {
    case LuaType::Number:
      if (lua_isinteger(state, index))
        *out = base::Value(static_cast<int>(lua_tointeger(state, index)));
      else
        *out = base::Value(lua_tonumber(state, index));
      break;
    case LuaType::Boolean:
      *out = base::Value(lua_toboolean(state, index));
      break;
    case LuaType::String:
      *out = base::Value(lua_tostring(state, index));
      break;
    case LuaType::Table: {
      size_t size = 0;
      if (IsTableArray(state, index, &size)) {
        std::vector<base::Value> storage;
        if (!lua::To(state, index, &storage))
          return false;
        *out = base::Value(std::move(storage));
      } else {
        *out = base::Value(base::Value::Type::DICTIONARY);
        auto* dict = static_cast<base::DictionaryValue*>(out);
        StackAutoReset reset(state);
        lua_pushnil(state);
        while (lua_next(state, index) != 0) {
          std::string key;
          base::Value value;
          if (!lua::To(state, -2, &key) || !lua::To(state, -1, &value))
            return false;
          lua_pop(state, 1);
          auto vval = std::make_unique<base::Value>(std::move(value));
          dict->SetWithoutPathExpansion(key.c_str(), std::move(vval));
        }
      }
      break;
    }
    default:
      *out = base::Value();
      break;
  }
  return true;;
}

// static
void Type<base::Value*>::Push(State* state, const base::Value* value) {
  return Type<base::Value>::Push(state, *value);
}

}  // namespace lua
