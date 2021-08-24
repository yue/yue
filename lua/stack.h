// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
//
// APIs to manipulate lua's stack.

#ifndef LUA_STACK_H_
#define LUA_STACK_H_

#include <functional>
#include <ostream>
#include <tuple>
#include <utility>

#include "base/check_op.h"
#include "lua/template_util.h"
#include "lua/types.h"

namespace lua {

// Think wrapper of lua_absindex.
inline int AbsIndex(State* state, int index) {
  return lua_absindex(state, index);
}

// Function template for Type<ArgType>::Push.
template<typename ArgType>
inline void Push(State* state, ArgType&& arg) {
  Type<typename internal::CallbackParamTraits<ArgType>::LocalType>::Push(
      state, std::forward<ArgType>(arg));
}

// Optimized version for string iterals.
template<size_t n>
inline void Push(State* state, const char (&str)[n]) {
  Type<const char[n]>::Push(state, str);
}

// Thin wrapper for lua_pushvalue.
struct ValueOnStack {
  ValueOnStack(State* state, int index) : index(AbsIndex(state, index)) {}
  int index;
};
inline void Push(State* state, ValueOnStack value) {
  lua_pushvalue(state, value.index);
}

// Thin wrapper for lua_upvalueindex.
struct UpValue {
  explicit UpValue(int index) : index(index) {}
  int index;
};
inline void Push(State* state, UpValue value) {
  lua_pushvalue(state, lua_upvalueindex(value.index));
}

// Thin wrapper for lua_pushcfunction.
struct CFunction {
  explicit CFunction(lua_CFunction func) : func(func) {}
  lua_CFunction func;
};
inline void Push(State* state, CFunction func) {
  lua_pushcfunction(state, func.func);
}

// Thin wrapper for lua_pushcclosure.
struct CClosure {
  CClosure(State* state, lua_CFunction func, int n)
      : func(func), n(n), top(GetTop(state)) {}
  lua_CFunction func;
  int n;
  int top;
};
inline void Push(State* state, CClosure closure) {
  for (int n = closure.n - 1; n >= 0; n--)
    lua_pushvalue(state, closure.top - n);
  lua_pushcclosure(state, closure.func, closure.n);
}

// Certain template functions are pushing nothing.
inline void Push(State* state) {
}

// Enable push arbitrary args at the same time.
template<typename ArgType, typename... ArgTypes>
inline void Push(State* state, ArgType&& arg, ArgTypes&&... args) {
  Push(state, std::forward<ArgType>(arg));
  Push(state, std::forward<ArgTypes>(args)...);
}

// The helper function for the tuple version of Push.
template<typename Tuple, size_t... Indices>
inline void Push(State* state, Tuple&& packed,
                 internal::IndicesHolder<Indices...>) {
  Push(state, std::get<Indices>(std::forward<Tuple>(packed))...);
}

// Treat std::tuple as unpacked args.
template<typename... ArgTypes>
inline void Push(State* state, std::tuple<ArgTypes...>&& packed) {
  Push(state, std::forward<std::tuple<ArgTypes...>>(packed),
       typename internal::IndicesGenerator<sizeof...(ArgTypes)>::type());
}

// Helper to push nil.
inline void PushNil(State* state) {
  lua_pushnil(state);
}

// Helpers for pushing strings.
PRINTF_FORMAT(2, 3)
inline void PushFormatedString(State* state,
                               const char* format,
                               ...)  {
  va_list ap;
  va_start(ap, format);
  lua_pushvfstring(state, format, ap);
  va_end(ap);
}

// Needed by the arbitrary length version of toxxx.
inline bool To(State* state, int index) {
  return true;
}

// Enable getting arbitrary args at the same time.
template<typename ArgType, typename... ArgTypes>
inline bool To(State* state, int index, ArgType* arg, ArgTypes&&... args) {
  return Type<ArgType>::To(state, index, arg) &&
         To(state, index + 1, std::forward<ArgTypes>(args)...);
}

// The helper function for the tuple version of To.
template<typename Tuple, size_t... Indices>
inline bool To(State* state, int index, Tuple* out,
               internal::IndicesHolder<Indices...>) {
  return To(state, index, &std::get<Indices>(*out)...);
}

// Get multiple values from stack.
template<typename... ArgTypes>
inline bool To(State* state, int index, std::tuple<ArgTypes...>* out) {
  return To(state, index, out,
            typename internal::IndicesGenerator<sizeof...(ArgTypes)>::type());
}

// Thin wrapper for lua_pop.
inline void PopAndIgnore(State* state, size_t n) {
  lua_pop(state, static_cast<int>(n));
}

// Get the values and pop them from statck.
template<typename T>
inline bool Pop(State* state, T* result) {
  if (To(state, -Values<T>::count, result)) {
    PopAndIgnore(state, Values<T>::count);
    return true;
  } else {
    return false;
  }
}

// Enable poping arbitrary args at the same time.
template<typename... ArgTypes>
inline bool Pop(State* state, ArgTypes... args) {
  if (To(state, -static_cast<int>(sizeof...(args)), args...)) {
    PopAndIgnore(state, sizeof...(args));
    return true;
  } else {
    return false;
  }
}

// Thin wrapper of lua_compare.
enum class CompareOp {
  EQ = LUA_OPEQ,
  LT = LUA_OPLT,
  LE = LUA_OPLE,
};
inline bool Compare(State* state, int index1, int index2, CompareOp op) {
  return lua_compare(state, index1, index2, static_cast<int>(op)) == 1;
}

// Thin wrapper of lua_insert.
inline void Insert(State* state, int index) {
  DCHECK_GE(index, 0) << "lua_insert does not accept pseudo-index";
  lua_insert(state, index);
}

}  // namespace lua

#endif  // LUA_STACK_H_
