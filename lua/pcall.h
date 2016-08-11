// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
//
// Extending the APIs for base::Callback.

#ifndef LUA_PCALL_H_
#define LUA_PCALL_H_

#include "base/bind.h"
#include "lua/pcall_internal.h"

namespace lua {

// Define how callbacks are converted.
template<typename Sig>
struct Type<base::Callback<Sig>> {
  static constexpr const char* name = "function";
  static inline void Push(State* state, const base::Callback<Sig>& callback) {
    internal::PushCFunction(state, callback);
  }
};

// Specialize for native functions.
template<typename T>
struct Type<T, typename std::enable_if<
                   internal::is_function_pointer<T>::value>::type> {
  static constexpr const char* name = "function";
  static inline void Push(State* state, T callback) {
    internal::PushCFunction(state, base::Bind(callback));
  }
};

// Specialize for member function.
template<typename T>
struct Type<T, typename std::enable_if<
                   std::is_member_function_pointer<T>::value>::type> {
  static constexpr const char* name = "method";
  static inline void Push(State* state, T callback) {
    int flags = internal::HolderIsFirstArgument;
    internal::PushCFunction(state, callback, flags);
  }
};

// Safely call the function on top of stack.
// When error happens, this API returns false and leaves the error on stack.
template<typename ReturnType, typename... ArgTypes>
inline bool PCall(State* state, ReturnType* result, ArgTypes... args) {
  Push(state, args...);

  if (lua_pcall(state, sizeof...(ArgTypes), Values<ReturnType>::count,
                0 /* no message handler */) != LUA_OK)
    return false;

  return Pop(state, result);
}

// Passing nullptr means there is no result expected.
template<typename... ArgTypes>
inline bool PCall(State* state, nullptr_t, ArgTypes... args) {
  Push(state, args...);
  return lua_pcall(state, sizeof...(ArgTypes), 0, 0) == LUA_OK;
}

}  // namespace lua

#endif  // LUA_PCALL_H_
