// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_TABLE_INTERNAL_H_
#define LUA_TABLE_INTERNAL_H_

#include <tuple>

#include "lua/stack.h"

namespace lua {

namespace internal {

// The generic version of lua_settable.
template<typename Key, typename Value>
inline void UnsafeSet(State* state, int index, const Key& key,
                      const Value& value) {
  Push(state, key, value);
  lua_settable(state, index);
}

// Optimize for lua_seti.
template<typename Value>
inline void UnsafeSet(State* state, int index, int key, const Value& value) {
  Push(state, value);
  lua_seti(state, index, key);
}

// Optimize for lua_setfield.
template<typename Value>
inline void UnsafeSet(State* state, int index, const char* key,
                      const Value& value) {
  Push(state, value);
  lua_setfield(state, index, key);
}

// Allow setting arbitrary key/value pairs.
template<typename Key, typename Value, typename... ArgTypes>
inline void UnsafeSet(State* state, int index, const Key& key,
                      const Value& value, const ArgTypes&... args) {
  UnsafeSet(state, index, key, value);
  UnsafeSet(state, index, args...);
}

// The helper function for the tuple version of UnsafeSet.
template<typename Tuple, size_t... Indices>
inline void UnsafeSet(State* state, int index, const Tuple& packed,
                      internal::IndicesHolder<Indices...>) {
  return UnsafeSet(state, index, std::get<Indices>(packed)...);
}

// The tuple version of UnsafeSet.
template<typename... ArgTypes>
inline void UnsafeSet(State* state, int index,
                      const std::tuple<ArgTypes...>& packed) {
  return UnsafeSet(
      state, index, packed,
      typename internal::IndicesGenerator<sizeof...(ArgTypes)>::type());
}

// The wrapper used by Set to call UnsafeSet.
template<typename... ArgTypes>
int UnsafeSetWrapper(State* state) {
  const std::tuple<const ArgTypes&...>& args =
      *static_cast<std::tuple<const ArgTypes&...>*>(lua_touserdata(state, 1));
  UnsafeSet(state, 2, args);
  return 0;
}

}  // namespace internal

}  // namespace lua

#endif  // LUA_TABLE_INTERNAL_H_
