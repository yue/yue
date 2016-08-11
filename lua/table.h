// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
//
// Helper functions to manipulate tables.

#ifndef LUA_TABLE_H_
#define LUA_TABLE_H_

#include <tuple>

#include "lua/table_internal.h"
#include "lua/stack.h"

namespace lua {

// Thin wrapper of lua_newtable.
inline void PushNewTable(State* state) {
  lua_newtable(state);
}

// The generic version of lua_rawset.
template<typename Key, typename Value>
inline void RawSet(State* state, int index, const Key& key,
                   const Value& value) {
  Push(state, key, value);
  lua_rawset(state, index);
}

// Optimize for lua_rawseti.
template<typename Value>
inline void RawSet(State* state, int index, int key, const Value& value) {
  Push(state, value);
  lua_rawseti(state, index, key);
}

// Allow setting arbitrary key/value pairs.
template<typename Key, typename Value, typename... ArgTypes>
inline void RawSet(State* state, int index, const Key& key, const Value& value,
                   const ArgTypes&... args) {
  RawSet(state, index, key, value);
  RawSet(state, index, args...);
}

// Generic version of lua_rawget.
template<typename Key>
inline void RawGet(State* state, int index, const Key& key) {
  Push(state, key);
  lua_rawget(state, index);
}

// Optimize for lua_rawgeti.
inline void RawGet(State* state, int index, int key) {
  lua_rawgeti(state, index, key);
}

// Allow getting arbitrary values.
template<typename Key, typename... ArgTypes>
inline void RawGet(State* state, int index, const Key& key,
                   const ArgTypes&... args) {
  RawGet(state, index, key);
  RawGet(state, index, args...);
}

// Helper function: Call RawGet for all keys and ignore the out.
template<typename Key, typename Value, typename... ArgTypes>
inline void RawGetKeyPairHelper(State* state, int index, const Key& key,
                                Value* out) {
  RawGet(state, index, key);
}
template<typename Key, typename Value, typename... ArgTypes>
inline void RawGetKeyPairHelper(State* state, int index, const Key& key,
                                Value* out, const ArgTypes&... args) {
  RawGetKeyPairHelper(state, index, key, out);
  RawGetKeyPairHelper(state, index, args...);
}

// Helper function: Call To for all values and ignore the key.
template<typename Key, typename Value>
inline bool ToKeyPairHelper(State* state, int index, const Key& key,
                            Value* out) {
  return To(state, index, out);
}
template<typename Key, typename Value, typename... ArgTypes>
inline bool ToKeyPairHelper(State* state, int index, const Key& key, Value* out,
                            const ArgTypes&... args) {
  return ToKeyPairHelper(state, index, key, out) &&
         ToKeyPairHelper(state, index + 1, args...);
}

// Allow getting and poping arbitrary values.
template<typename Key, typename Value, typename... ArgTypes>
inline bool RawGetAndPop(State* state, int index, const Key& key, Value* out,
                         const ArgTypes&... args) {
  int current_top = GetTop(state);
  RawGetKeyPairHelper(state, index, key, out, args...);
  bool success = ToKeyPairHelper(state, current_top + 1, key, out, args...);
  SetTop(state, current_top);
  return success;
}

// The safe wrapper for the unsafe lua_settable.
// When failed, false is returned and the error is left on stack.
template<typename... ArgTypes>
inline bool Set(State* state, int index, const ArgTypes&... args) {
  std::tuple<const ArgTypes&...> args_refs(args...);
  lua_pushcfunction(state, &internal::UnsafeSetWrapper<const ArgTypes&...>);
  lua_pushlightuserdata(state, &args_refs);
  lua_pushvalue(state, index);
  return lua_pcall(state, 2, 0, 0) == LUA_OK;
}

// The safe wrapper for the unsafe lua_gettable.
// When failed, false is returned and the error is left on stack.
template<typename... ArgTypes>
inline bool Get(State* state, int index, const ArgTypes&... args) {
  std::tuple<const ArgTypes&...> args_refs(args...);
  lua_pushcfunction(state, &internal::UnsafeGetWrapper<const ArgTypes&...>);
  lua_pushlightuserdata(state, &args_refs);
  lua_pushvalue(state, index);
  return lua_pcall(state, 2, sizeof...(ArgTypes), 0) == LUA_OK;
}

// Use Get to receive table members and pop them out of stack.
// When failed, false is returned and the error is left on stack.
template<typename... ArgTypes>
inline bool GetAndPop(State* state, int index, const ArgTypes&... args) {
  std::tuple<const ArgTypes&...> args_refs(args...);
  int current_top = GetTop(state);
  lua_pushcfunction(state,
                    &internal::UnsafeGetAndPopWrapper<const ArgTypes&...>);
  lua_pushlightuserdata(state, &args_refs);
  lua_pushvalue(state, index);
  if (lua_pcall(state, 2, sizeof...(ArgTypes) / 2, 0) != LUA_OK)
    return false;
  bool success = ToKeyPairHelper(state, current_top + 1, args...);
  SetTop(state, current_top);
  if (!success)  // Push an error to match the behavior of pcall.
    Push(state, "error converting values");
  return success;
}

}  // namespace lua

#endif  // LUA_TABLE_H_
