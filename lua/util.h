// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_UTIL_H_
#define LUA_UTIL_H_

namespace lua {

template<typename T, typename... ArgTypes>
T* CreateOnHeap(ArgTypes... args) {
  return new T(args...);
}

template<typename T, typename... ArgTypes>
T CreateOnStack(ArgTypes... args) {
  return T(args...);
}

}  // namespace lua

#endif  // LUA_UTIL_H_
