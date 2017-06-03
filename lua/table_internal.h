// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_TABLE_INTERNAL_H_
#define LUA_TABLE_INTERNAL_H_

#include <tuple>
#include <utility>

#include "lua/stack.h"

namespace lua {

namespace internal {

// The generic version of lua_settable.
template<typename Key, typename Value>
inline void UnsafeSet(State* state, int index, const Key& key,
                      const Value& value) {
  index = AbsIndex(state, index);
  Push(state, key, value);
  lua_settable(state, index);
}

// Optimize for lua_seti.
template<typename Value>
inline void UnsafeSet(State* state, int index, int key, const Value& value) {
  index = AbsIndex(state, index);
  Push(state, value);
  lua_seti(state, index, key);
}

// Optimize for lua_setfield.
template<typename Value>
inline void UnsafeSet(State* state, int index, const char* key,
                      const Value& value) {
  index = AbsIndex(state, index);
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
  const auto& args =
      *static_cast<std::tuple<ArgTypes...>*>(lua_touserdata(state, 1));
  UnsafeSet(state, 2, args);
  return 0;
}

// Create a tuple but replaces ValueOnStack with UpValue.
template<typename T>
struct PSetArgReplacer {
  using Type = T;
  static inline Type Do(State* state, int* upvalues, T arg) {
    static_assert(std::is_reference<T>::value,
                  "Type passed to PSetArgReplacer must be reference type");
    return arg;
  }
};

template<>
struct PSetArgReplacer<const ValueOnStack&> {
  using Type = UpValue;
  static inline Type Do(State* state, int* upvalues, const ValueOnStack& arg) {
    Push(state, arg);
    return UpValue(++(*upvalues));
  }
};

template<typename... ArgTypes>
struct PSetArgs {
  using Tuple = std::tuple<typename PSetArgReplacer<const ArgTypes&>::Type...>;
  static Tuple Convert(State* state, int* upvalues, const ArgTypes&... args) {
    Tuple refs(std::forward<typename PSetArgReplacer<const ArgTypes&>::Type>(
        PSetArgReplacer<const ArgTypes&>::Do(state, upvalues, args))...);
    return std::move(refs);
  }
};

// The generic version of lua_gettable.
template<typename Key>
inline void UnsafeGet(State* state, int index, const Key& key) {
  index = AbsIndex(state, index);
  Push(state, key);
  lua_gettable(state, index);
}

// Get and put on stack.
inline void UnsafeGet(State* state, int index) {
  lua_gettable(state, index);
}

// Optimize for lua_geti.
template<typename Value>
inline void UnsafeGet(State* state, int index, int key) {
  lua_geti(state, index, key);
}

// Optimize for lua_getfield.
template<typename Value>
inline void UnsafeGet(State* state, int index, const char* key) {
  lua_getfield(state, index, key);
}

// Allow getting arbitrary keys.
template<typename Key, typename... ArgTypes>
inline void UnsafeGet(State* state, int index, const Key& key,
                      const ArgTypes&... args) {
  UnsafeGet(state, index, key);
  UnsafeGet(state, index, args...);
}

// The helper function for the tuple version of UnsafeGet.
template<typename Tuple, size_t... Indices>
inline void UnsafeGet(State* state, int index, const Tuple& packed,
                      internal::IndicesHolder<Indices...>) {
  return UnsafeGet(state, index, std::get<Indices>(packed)...);
}

// The tuple version of UnsafeGet.
template<typename... ArgTypes>
inline void UnsafeGet(State* state, int index,
                      const std::tuple<ArgTypes...>& packed) {
  return UnsafeGet(
      state, index, packed,
      typename internal::IndicesGenerator<sizeof...(ArgTypes)>::type());
}

// The wrapper used by Get to call UnsafeGet.
template<typename... ArgTypes>
inline int UnsafeGetWrapper(State* state) {
  const std::tuple<const ArgTypes&...>& args =
      *static_cast<std::tuple<const ArgTypes&...>*>(lua_touserdata(state, 1));
  UnsafeGet(state, 2, args);
  return sizeof...(ArgTypes);
}

// Strip the out args from args and feed the keys to UnsafeGet.
inline void UnsafeGetKeyPairHelper(State* state, int index) {
}
template<typename Key, typename Value, typename... ArgTypes>
inline void UnsafeGetKeyPairHelper(State* state, int index, const Key& key,
                                   Value* out, const ArgTypes&... args) {
  UnsafeGet(state, index, key);
  UnsafeGetKeyPairHelper(state, index, args...);
}

// Tuple version of UnsafeGetKeyPairHelper.
template<typename Tuple, size_t... Indices>
inline void UnsafeGetKeyPairHelper(State* state, int index, const Tuple& packed,
                                   internal::IndicesHolder<Indices...>) {
  UnsafeGetKeyPairHelper(state, index, std::get<Indices>(packed)...);
}
template<typename... ArgTypes>
inline void UnsafeGetKeyPairHelper(State* state, int index,
                                   const std::tuple<ArgTypes...>& packed) {
  UnsafeGetKeyPairHelper(
      state, index, packed,
      typename internal::IndicesGenerator<sizeof...(ArgTypes)>::type());
}

// The wrapper used by GetAndPop.
template<typename... ArgTypes>
inline int UnsafeGetAndPopWrapper(State* state) {
  const std::tuple<const ArgTypes&...>& args =
      *static_cast<std::tuple<const ArgTypes&...>*>(lua_touserdata(state, 1));
  UnsafeGetKeyPairHelper(state, 2, args);
  return sizeof...(ArgTypes) / 2;
}

}  // namespace internal

}  // namespace lua

#endif  // LUA_TABLE_INTERNAL_H_
