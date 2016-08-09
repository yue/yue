// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef LUA_CALLBACK_INTERNAL_H_
#define LUA_CALLBACK_INTERNAL_H_

#include "base/callback.h"
#include "lua/to.h"

namespace lua {

namespace internal {

// DispatchToCallback converts all the lua arguments to C++ types and
// invokes the base::Callback.
template <typename Sig>
struct Dispatcher {};

template <typename ReturnType, typename... ArgTypes>
struct Dispatcher<ReturnType(ArgTypes...)> {
  static int DispatchToCallback(State* state) {
    return 0;
  }
};

// Push the function on stack without wrapping it with pcall.
template<typename Sig>
inline bool PushCFunction(State* state, const base::Callback<Sig>& callback) {
  lua_pushcfunction(state,
                    &internal::Dispatcher<Sig>::DispatchToCallback);
  return false;
}

// Get how many values would be returned for the type.
template<typename ReturnType>
struct ReturnValues {
  static const int count = 1;
};

template<>
struct ReturnValues<void> {
  static const int count = 0;
};

}  // namespace internal

}  // namespace lua

#endif  // LUA_CALLBACK_INTERNAL_H_
