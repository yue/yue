// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_USER_DATA_H_
#define LUA_USER_DATA_H_

#include "lua/state.h"

namespace lua {

// Defines how a type is converted to userdata.
template<typename T, typename Enable = void>
struct UserData {
  using Type = T;
  static inline void Destruct(Type* data) {
    data->~Type();
  }
};

// Generic callback for __gc.
template<typename T>
int OnGC(lua::State* state) {
  void* data = lua_touserdata(state, 1);
  UserData<T>::Destruct(static_cast<typename UserData<T>::Type*>(data));
  return 0;
}

}  // namespace lua

#endif  // LUA_USER_DATA_H_
