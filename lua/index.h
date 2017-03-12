// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_INDEX_H_
#define LUA_INDEX_H_

#include "lua/table.h"
#include "lua/user_data.h"

namespace lua {

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

}  // namespace lua

#endif  // LUA_INDEX_H_
