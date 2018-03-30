// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
//
// Extending the APIs for std::function.

#ifndef LUA_CALLBACK_H_
#define LUA_CALLBACK_H_

#include <memory>
#include <utility>

#include "lua/callback_internal.h"

namespace lua {

// Define how callbacks are converted.
// Note that lua callbacks are always stored as weak reference when converted to
// C++ function, otherwise we would have unresolvable cyclic reference leading
// to memory leak.
template<typename ReturnType, typename... ArgTypes>
struct Type<std::function<ReturnType(ArgTypes...)>> {
  static constexpr const char* name = "function";
  static inline void Push(
      State* state,
      const std::function<ReturnType(ArgTypes...)>& callback) {
    internal::PushCFunction(state, callback);
  }
  static inline bool To(State* state, int index,
                        std::function<ReturnType(ArgTypes...)>* out) {
    if (GetType(state, index) == LuaType::Nil) {
      *out = nullptr;
      return true;
    }
    if (GetType(state, index) != LuaType::Function)
      return false;
    std::shared_ptr<Handle> handle = Weak::New(state, index);
    *out = [state, handle](ArgTypes... args) -> ReturnType {
      return internal::PCallHelper<ReturnType, ArgTypes...>::Run(
          state, handle, std::move(args)...);
    };
    return true;
  }
};

// Specialize for native functions.
template<typename T>
struct Type<T, typename std::enable_if<
                   internal::is_function_pointer<T>::value>::type> {
  static constexpr const char* name = "function";
  static inline void Push(State* state, T callback) {
    using RunType = typename internal::FunctorTraits<T>::RunType;
    internal::PushCFunction(state, std::function<RunType>(callback));
  }
};

// Specialize for member function.
template<typename T>
struct Type<T, typename std::enable_if<
                   std::is_member_function_pointer<T>::value>::type> {
  static constexpr const char* name = "method";
  static inline void Push(State* state, T callback) {
    using RunType = typename internal::FunctorTraits<T>::RunType;
    internal::PushCFunction(state, std::function<RunType>(callback));
  }
};

}  // namespace lua

#endif  // LUA_CALLBACK_H_
