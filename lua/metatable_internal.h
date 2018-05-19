// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_METATABLE_INTERNAL_H_
#define LUA_METATABLE_INTERNAL_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "lua/table.h"
#include "lua/user_data.h"

namespace lua {

namespace internal {

// Read a |key| from weak wrapper table and put the wrapper on stack.
// Return false when there is no such |key| in table.
bool WrapperTableGet(State* state, void* key);

// Save a wrapper at |index| to weak wrapper table with |key|.
void WrapperTableSet(State* state, void* key, int index);

// A implementation of __index that works as prototype chain.
int InheritanceChainLookup(State* state);

// A implementation of __newindex that works as prototype chain.
int InheritanceChainAssign(State* state);

// Dereference the native object for __gc.
template<typename T>
int DereferenceOnGC(lua::State* state) {
  void* data = lua_touserdata(state, 1);
  UserData<T>::Destruct(static_cast<typename UserData<T>::Type*>(data));
  return 0;
}

// Create metatable for T, returns true if the metattable has already been
// created.
template<typename T>
bool NewMetaTable(State* state) {
  // Each type would get its own storage of |key| variable, and we use its
  // address as the key to the type's metattable.
  static int key = 0xFEE;
  RawGet(state, LUA_REGISTRYINDEX, static_cast<void*>(&key));
  if (GetType(state, -1) != LuaType::Nil)
    return true;

  // Like luaL_newmetatable, but use pointer as key.
  PopAndIgnore(state, 1);
  NewTable(state, 0, 2);
  RawSet(state, -1, "__name", static_cast<const char*>(Type<T>::name));
  RawSet(state, LUA_REGISTRYINDEX, static_cast<void*>(&key),
         ValueOnStack(state, -1));

  RawSet(state, -1,
         "__gc", CFunction(&DereferenceOnGC<T>),
         "__index", CClosure(state, &InheritanceChainLookup, 1),
         "__newindex", CClosure(state, &InheritanceChainAssign, 1));
  Type<T>::BuildMetaTable(state, AbsIndex(state, -1));
  return false;
}

// Create metattable inheritance chain for T and its BaseTypes.
template<typename T, typename Enable = void>
struct InheritanceChain {
  // There is no base type.
  static inline void Push(State* state) {
    NewMetaTable<T>(state);
  }
};

template<typename T>
struct InheritanceChain<T, typename std::enable_if<std::is_class<
                               typename Type<T>::base>::value>::type> {
  static inline void Push(State* state) {
    if (NewMetaTable<T>(state))  // already created.
      return;

    // Inherit from base type's metatable.
    StackAutoReset reset(state);
    InheritanceChain<typename Type<T>::base>::Push(state);
    RawSet(state, -2, "__super", ValueOnStack(state, -1));
  }
};

}  // namespace internal

}  // namespace lua

#endif  // LUA_METATABLE_INTERNAL_H_
