// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
//
// Extending the APIs for base::Callback.

#ifndef LUA_CALLBACK_H_
#define LUA_CALLBACK_H_

#include <utility>

#include "base/bind.h"
#include "lua/callback_internal.h"

namespace lua {

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
    if (GetType(state, index) == LuaType::Nil) {
      *out = base::Callback<ReturnType(ArgTypes...)>();
      return true;
    }
    if (GetType(state, index) != LuaType::Function)
      return false;
    *out = base::Bind(&internal::PCallHelper<ReturnType, ArgTypes...>::Run,
                      state, Persistent::New(state, index));
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
    internal::PushCFunction(state, base::Bind(callback));
  }
};

// Helper to push a type directly by invoking its constructor.
template<typename T, typename... ArgTypes>
struct NativeConstructor {
  static T Call(ArgTypes... args) {
    return std::move(T(args...));
  }
};
template<typename T, typename... ArgTypes>
inline void Push(State* state, NativeConstructor<T, ArgTypes...>) {
  internal::PushCFunction(state,
                          base::Bind(NativeConstructor<T, ArgTypes...>::Call));
}

}  // namespace lua

#endif  // LUA_CALLBACK_H_
