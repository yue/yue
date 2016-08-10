// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
//
// Helper functions to manipulate tables.

#ifndef LUA_TABLE_H_
#define LUA_TABLE_H_

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
                   ArgTypes... args) {
  RawSet(state, index, key, value);
  RawSet(state, index, args...);
}

// Generic version of lua_rawget.
template<typename Key>
inline LuaType RawGet(State* state, int index, const Key& key) {
  Push(state, key);
  return static_cast<LuaType>(lua_rawget(state, index));
}

// Optimize for lua_rawgeti.
inline LuaType RawGet(State* state, int index, int key) {
  return static_cast<LuaType>(lua_rawgeti(state, index, key));
}

// Allow getting arbitrary values.
template<typename Key, typename... ArgTypes>
inline void RawGet(State* state, int index, const Key& key, ArgTypes... args) {
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
                                Value* out, ArgTypes... args) {
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
                            ArgTypes... args) {
  return ToKeyPairHelper(state, index, key, out) &&
         ToKeyPairHelper(state, index + 1, args...);
}

// Allow getting and poping arbitrary values.
template<typename Key, typename Value, typename... ArgTypes>
inline bool RawGetAndPop(State* state, int index, const Key& key, Value* out,
                         ArgTypes... args) {
  int current_top = GetTop(state);
  RawGetKeyPairHelper(state, index, key, out, args...);
  bool success = ToKeyPairHelper(state, current_top + 1, key, out, args...);
  SetTop(state, current_top);
  return success;
}

}  // namespace lua

#endif  // LUA_TABLE_H_
