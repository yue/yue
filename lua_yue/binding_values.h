// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_YUE_BINDING_VALUES_H_
#define LUA_YUE_BINDING_VALUES_H_

#include "base/values.h"
#include "lua/lua.h"

namespace lua {

template<>
struct Type<base::Value> {
  static constexpr const char* name = "Value";
  static void Push(State* state, const base::Value& value);
  static bool To(State* state, int index, base::Value* out);
};

template<>
struct Type<base::Value*> {
  static constexpr const char* name = "Value";
  static void Push(State* state, const base::Value* value);
};

}  // namespace lua

#endif  // LUA_YUE_BINDING_VALUES_H_
