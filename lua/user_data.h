// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_USER_DATA_H_
#define LUA_USER_DATA_H_

#include "lua/state.h"
#include "lua/types.h"

namespace lua {

// Defines how a type is converted to userdata.
template<typename T, typename Enable = void>
struct UserData {
  using Type = T;
  static inline void Construct(Type* data, T* ptr) {
    *data = *ptr;
  }
  static inline void Destruct(Type* data) {
    data->~Type();
  }
  static inline T* From(Type* data) {
    return data;
  }
};

// Generic callback for __gc.
template<typename T>
int OnGC(lua::State* state) {
  void* data = lua_touserdata(state, 1);
  UserData<T>::Destruct(static_cast<typename UserData<T>::Type*>(data));
  return 0;
}

// Pushing a new UserData from |ptr|.
template<typename T>
void NewUserData(State* state, T* ptr) {
  using Type = typename UserData<T>::Type;
  Type* memory = static_cast<Type*>(lua_newuserdata(state, sizeof(Type)));
  UserData<T>::Construct(memory, ptr);
}

// Construct a new UserData with |T|, and assign it with a default metatable.
template<typename T, typename... ArgTypes>
void NewUserData(State* state, const ArgTypes... args) {
  using Type = typename UserData<T>::Type;
  Type* memory = static_cast<Type*>(lua_newuserdata(state, sizeof(Type)));
  new(memory) T(args...);
  NewTable(state, 0, 1);
  RawSet(state, -1, "__gc", CFunction(&OnGC<T>));
  SetMetaTable(state, -2);
}

}  // namespace lua

#endif  // LUA_USER_DATA_H_
