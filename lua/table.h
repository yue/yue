// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
//
// Helper functions to manipulate tables.

#ifndef LUA_TABLE_H_
#define LUA_TABLE_H_

#include <tuple>
#include <utility>

#include "lua/table_internal.h"

namespace lua {

// Thin wrapper of lua_createtable.
inline void NewTable(State* state, int narr = 0, int nrec = 0) {
  lua_createtable(state, narr, nrec);
}

// Get length of table (or any other value).
inline size_t RawLen(State* state, int index) {
  return lua_rawlen(state, index);
}

// Thin wrappers of lua_getmetatable and lua_setmetatable.
inline void SetMetaTable(State* state, int index) {
  lua_setmetatable(state, index);
}
inline bool GetMetaTable(State* state, int index) {
  return lua_getmetatable(state, index) == 1;
}

// The generic version of lua_rawset.
template<typename Key, typename Value>
inline void RawSet(State* state, int index, Key&& key, Value&& value) {
  index = AbsIndex(state, index);
  Push(state, std::forward<Key>(key), std::forward<Value>(value));
  lua_rawset(state, index);
}

// Optimize for lua_rawseti.
template<typename Value>
inline void RawSet(State* state, int index, int key, Value&& value) {
  index = AbsIndex(state, index);
  Push(state, std::forward<Value>(value));
  lua_rawseti(state, index, key);
}

// Optimize for lua_rawsetp.
template<typename Value>
inline void RawSet(State* state, int index, void* key, Value&& value) {
  index = AbsIndex(state, index);
  Push(state, std::forward<Value>(value));
  lua_rawsetp(state, index, key);
}

// Allow setting arbitrary key/value pairs.
template<typename Key, typename Value, typename... ArgTypes>
inline void RawSet(State* state, int index, Key&& key, Value&& value,
                   ArgTypes&&... args) {
  RawSet(state, index, std::forward<Key>(key), std::forward<Value>(value));
  RawSet(state, index, std::forward<ArgTypes>(args)...);
}

// Generic version of lua_rawget.
template<typename Key>
inline void RawGet(State* state, int index, Key&& key) {
  index = AbsIndex(state, index);
  Push(state, std::forward<Key>(key));
  lua_rawget(state, index);
}

// Optimize for lua_rawgeti.
inline void RawGet(State* state, int index, int key) {
  index = AbsIndex(state, index);
  lua_rawgeti(state, index, key);
}

// Optimize for lua_rawgetp.
inline void RawGet(State* state, int index, void* key) {
  index = AbsIndex(state, index);
  lua_rawgetp(state, index, key);
}

// Allow getting arbitrary values.
template<typename Key, typename... ArgTypes>
inline void RawGet(State* state, int index, Key&& key,
                   ArgTypes&&... args) {
  index = AbsIndex(state, index);
  RawGet(state, index, std::forward<Key>(key));
  RawGet(state, index, std::forward<ArgTypes>(args)...);
}

// Helper function: Call RawGet for all keys and ignore the out.
template<typename Key, typename Value, typename... ArgTypes>
inline bool RawGetKeyPairHelper(State* state, int index, const Key& key,
                                Value* out) {
  RawGet(state, index, key);
  return GetType(state, -1) != LuaType::Nil;
}
template<typename Key, typename Value, typename... ArgTypes>
inline bool RawGetKeyPairHelper(State* state, int index, const Key& key,
                                Value* out, ArgTypes&&... args) {
  index = AbsIndex(state, index);
  bool success = RawGetKeyPairHelper(state, index, key, out);
  success &= RawGetKeyPairHelper(state, index, std::forward<ArgTypes>(args)...);
  return success;
}

// Helper function: Call To for all values and ignore the key.
template<typename Key, typename Value>
inline bool ToKeyPairHelper(State* state, int index, const Key& key,
                            Value* out) {
  if (GetType(state, index) == LuaType::Nil)  // ignore unexist keys
    return true;
  return To(state, index, out);
}
template<typename Key, typename Value, typename... ArgTypes>
inline bool ToKeyPairHelper(State* state, int index, const Key& key, Value* out,
                            ArgTypes&&... args) {
  bool success = ToKeyPairHelper(state, index, key, out);
  success &= ToKeyPairHelper(state, index + 1, std::forward<ArgTypes>(args)...);
  return success;
}

// Allow getting and poping arbitrary values.
template<typename Key, typename Value, typename... ArgTypes>
inline bool RawGetAndPop(State* state, int index, const Key& key, Value* out,
                         const ArgTypes&... args) {
  StackAutoReset reset(state);
  return RawGetKeyPairHelper(state, index, key, out, args...) &&
         ToKeyPairHelper(state, reset.top() + 1, key, out, args...);
}

// Like RawGetAndPop, but ignore unexist keys.
template<typename Key, typename Value, typename... ArgTypes>
inline bool ReadOptions(State* state, int index, const Key& key, Value* out,
                        const ArgTypes&... args) {
  StackAutoReset reset(state);
  RawGetKeyPairHelper(state, index, key, out, args...);
  return ToKeyPairHelper(state, reset.top() + 1, key, out, args...);
}

// The safe wrapper for the unsafe lua_settable.
// When failed, false is returned and the error is left on stack.
template<typename... ArgTypes>
inline bool PSet(State* state, int index, const ArgTypes&... args) {
  index = AbsIndex(state, index);
  int upvalues = 0;
  auto args_refs =
      internal::PSetArgs<ArgTypes...>::Convert(state, &upvalues, args...);
  lua_pushcclosure(
      state,
      &internal::UnsafeSetWrapper<
          typename internal::PSetArgReplacer<const ArgTypes&>::Type...>,
      upvalues);
  lua_pushlightuserdata(state, &args_refs);
  lua_pushvalue(state, index);
  return lua_pcall(state, 2, 0, 0) == LUA_OK;
}

// The safe wrapper for the unsafe lua_gettable.
// When failed, false is returned and the error is left on stack.
template<typename... ArgTypes>
inline bool PGet(State* state, int index, const ArgTypes&... args) {
  index = AbsIndex(state, index);
  std::tuple<const ArgTypes&...> args_refs(args...);
  lua_pushcfunction(state, &internal::UnsafeGetWrapper<const ArgTypes&...>);
  lua_pushlightuserdata(state, &args_refs);
  lua_pushvalue(state, index);
  return lua_pcall(state, 2, sizeof...(ArgTypes), 0) == LUA_OK;
}

// Use Get to receive table members and pop them out of stack.
// When failed, false is returned and the error is left on stack.
template<typename... ArgTypes>
inline bool PGetAndPop(State* state, int index, const ArgTypes&... args) {
  index = AbsIndex(state, index);
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

// Helper to create weak tables.
inline void PushWeakTable(State* state, const char* name, const char* mode) {
  if (luaL_newmetatable(state, name)) {
    lua::NewTable(state);
    lua::RawSet(state, -1, "__mode", mode);
    lua::SetMetaTable(state, -2);
  }
}

// Return or create a table for key.
template<typename T>
void RawGetOrCreateTable(State* state, int table, const T& key) {
  table = AbsIndex(state, table);
  RawGet(state, table, key);
  if (GetType(state, -1) != LuaType::Table) {
    PopAndIgnore(state, 1);
    NewTable(state, 0, 1);
    RawSet(state, table, key, ValueOnStack(state, -1));
  }
}

// Attach a table to a variable.
void PushCustomDataTable(State* state, int key);

// Push a table that stores references.
void PushRefsTable(State* state, const char* name, int owner);

}  // namespace lua

#endif  // LUA_TABLE_H_
