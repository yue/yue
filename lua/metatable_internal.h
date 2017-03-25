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

// Create metatable for T, returns true if the metattable has already been
// created.
template<typename T>
bool NewMetaTable(State* state) {
  if (luaL_newmetatable(state, Type<T>::name) == 0)
    return true;

  RawSet(state, -1,
         "__gc", CFunction(&OnGC<T>),
         "__index", ValueOnStack(state, -1));
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
    InheritanceChain<typename Type<T>::base>::Push(state);
    NewTable(state, 0, 1);
    RawSet(state, -1, "__index", ValueOnStack(state, -2));
    SetMetaTable(state, -3);
    PopAndIgnore(state, 1);
  }
};

}  // namespace internal

}  // namespace lua

#endif  // LUA_METATABLE_INTERNAL_H_
