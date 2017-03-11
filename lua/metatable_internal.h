// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_METATABLE_INTERNAL_H_
#define LUA_METATABLE_INTERNAL_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "lua/table.h"

namespace lua {

// Defines how the wrapper of RefCounted is destructed.
template<typename T>
struct GCTratis<T, typename std::enable_if<std::is_base_of<
                       base::subtle::RefCountedBase, T>::value>::type> {
  static inline void Destruct(void* data) {
    (*static_cast<T**>(data))->Release();
  }
};

// Defines how the wrapper of WeakPtr is destructed.
template<typename T>
struct GCTratis<T, typename std::enable_if<std::is_base_of<
                       base::internal::WeakPtrBase,
                       decltype(((T*)nullptr)->GetWeakPtr())>::value>::type> {
  static inline void Destruct(void* data) {
    using WrapperType = base::WeakPtr<T>;
    static_cast<WrapperType*>(data)->~WrapperType();
  }
};

namespace internal {

// Read a |key| from weak wrapper table and put the wrapper on stack.
// Return false when there is no such |key| in table.
bool WrapperTableGet(State* state, void* key);

// Save a wrapper at |index| to weak wrapper table with |key|.
void WrapperTableSet(State* state, void* key, int index);

// The default __index handler which looks up in the metatable.
int DefaultPropertyLookup(State* state);

// Check whether T has an Index handler, if not use the default property lookup
// that searches in metatable.
template<typename T, typename Enable = void>
struct Indexer {
  static inline void Set(State* state, int index) {
    RawSet(state, index, "__index", ValueOnStack(state, index));
  }
};

template<typename T>
struct Indexer<T, typename std::enable_if<std::is_pointer<
                      decltype(&Type<T>::Index)>::value>::type> {
  static inline void Set(State* state, int index) {
    RawSet(state, index, "__index", CFunction(&Index));
  }
  static int Index(State* state) {
    int r = Type<T>::Index(state);
    if (r > 0)
      return r;
    // Go to the default routine.
    return DefaultPropertyLookup(state);
  }
};

// Check whether T has an NewIndex handler, if it does then set __newindex.
template<typename T, typename Enable = void>
struct NewIndexer {
  static inline void Set(State* state, int index) {
  }
};

template<typename T>
struct NewIndexer<T, typename std::enable_if<std::is_function<
                         decltype(Type<T>::NewIndex)>::value>::type> {
  static inline void Set(State* state, int index) {
    RawSet(state, index, "__newindex", CFunction(&NewIndex));
  }
  static int NewIndex(State* state) {
    int r = Type<T>::NewIndex(state);
    if (r > 0)
      return r;
    lua::Push(state, "unaccepted assignment");
    lua_error(state);
    return 0;
  }
};

// Create metatable for T, returns true if the metattable has already been
// created.
template<typename T>
bool PushSingleTypeMetaTable(State* state) {
  if (luaL_newmetatable(state, Type<T>::name) == 0)
    return true;

  RawSet(state, -1, "__gc", CFunction(&OnGC<T>));
  Indexer<T>::Set(state, -1);
  NewIndexer<T>::Set(state, -1);
  Type<T>::BuildMetaTable(state, -1);
  return false;
}

// Create metattable inheritance chain for T and its BaseTypes.
template<typename T, typename Enable = void>
struct InheritanceChain {
  // There is no base type.
  static inline void Push(State* state) {
    PushSingleTypeMetaTable<T>(state);
  }
};

template<typename T>
struct InheritanceChain<T, typename std::enable_if<std::is_class<
                               typename Type<T>::base>::value>::type> {
  static inline void Push(State* state) {
    if (PushSingleTypeMetaTable<T>(state))  // already created.
      return;

    // Inherit from base type's metatable.
    InheritanceChain<typename Type<T>::base>::Push(state);
    PushNewTable(state, 0, 1);
    RawSet(state, -1, "__index", ValueOnStack(state, -2));
    SetMetaTable(state, -3);
    PopAndIgnore(state, 1);
  }
};

}  // namespace internal

}  // namespace lua

#endif  // LUA_METATABLE_INTERNAL_H_
