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

// Safely call the function on top of stack.
// When error happens, this API returns false and leaves the error on stack.
template<typename ReturnType, typename... ArgTypes>
inline bool PCall(State* state, ReturnType* result, const ArgTypes&... args) {
  Push(state, args...);

  if (lua_pcall(state, sizeof...(ArgTypes), Values<ReturnType>::count,
                0 /* no message handler */) != LUA_OK)
    return false;

  return Pop(state, result);
}

// Passing nullptr means there is no result expected.
template<typename... ArgTypes>
inline bool PCall(State* state, nullptr_t, const ArgTypes&... args) {
  Push(state, args...);
  return lua_pcall(state, sizeof...(ArgTypes), 0, 0) == LUA_OK;
}

template<typename ReturnType, typename...ArgTypes>
ReturnType PCallHelper(State* state, const std::unique_ptr<Handle>& handle,
                       ArgTypes... args) {
  ReturnType result = ReturnType();
  handle->Push();
  PCall(state, &result, args...);
  return result;
}

// Define how callbacks are converted.
template<typename ReturnType, typename... ArgTypes>
struct Type<base::Callback<ReturnType(ArgTypes...)>> {
  static constexpr const char* name = "function";
  static inline void Push(
      State* state,
      const base::Callback<ReturnType(ArgTypes...)>& callback) {
    internal::PushCFunction(state, callback);
  }
  static inline bool To(State* state, int index,
                        base::Callback<ReturnType(ArgTypes...)>* out) {
    if (GetType(state, index) != LuaType::Function)
      return false;
    std::unique_ptr<Handle> handle = Handle::New(state, index);
    *out = base::Bind(&PCallHelper<ReturnType, ArgTypes...>, state,
                      std::move(handle));
    return true;
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

}  // namespace lua

#endif  // LUA_PCALL_H_
