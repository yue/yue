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
inline void RawSet(State* state, int index, Key key, Value value) {
  Push(state, key, value);
  lua_rawset(state, index);
}

// Optimize for lua_rawseti.
template<typename Value>
inline void RawSet(State* state, int index, int key, Value value) {
  Push(state, value);
  lua_rawseti(state, index, key);
}

// Allow setting arbitrary key/value pairs.
template<typename Key, typename Value, typename... ArgTypes>
inline void RawSet(State* state, int index, Key key, Value value,
                   ArgTypes... args) {
  RawSet(state, index, key, value);
  RawSet(state, index, args...);
}

// Thin wrapper of lua_rawget.

}  // namespace lua

#endif  // LUA_TABLE_H_
