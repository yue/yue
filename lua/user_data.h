// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_USER_DATA_H_
#define LUA_USER_DATA_H_

#include <utility>

#include "lua/state.h"
#include "lua/types.h"

namespace lua {

// Defines how a type is converted to userdata.
template<typename T, typename Enable = void>
struct UserData {
  using Type = T;
  static inline void Construct(State* state, Type* data, T* ptr) {
    *data = *ptr;
  }
  static inline void Destruct(Type* data) {
    data->~Type();
  }
  static inline T* From(State* state, void* data) {
    return static_cast<T*>(data);
  }
};

// Pushing a new UserData from |ptr|.
template<typename T>
void NewUserData(State* state, T* ptr) {
  using Type = typename UserData<T>::Type;
  Type* memory = static_cast<Type*>(lua_newuserdata(state, sizeof(Type)));
  UserData<T>::Construct(state, memory, ptr);
}

// Destruct the native object for __gc.
template<typename T>
int DestructOnGC(lua::State* state) {
  void* data = lua_touserdata(state, 1);
  static_cast<T*>(data)->~T();
  return 0;
}

// Construct a new UserData with |T|, and assign it with a default metatable.
// T must be type that can be destructed by calling destructor.
template<typename T, typename... ArgTypes>
void NewUserData(State* state, ArgTypes&&... args) {
  T* memory = static_cast<T*>(lua_newuserdata(state, sizeof(T)));
  new(memory) T(std::forward<ArgTypes>(args)...);
  NewTable(state, 0, 1);
  RawSet(state, -1, "__gc", CFunction(&DestructOnGC<T>));
  SetMetaTable(state, -2);
}

}  // namespace lua

#endif  // LUA_USER_DATA_H_
