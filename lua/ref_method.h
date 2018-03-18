// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_REF_METHOD_H_
#define LUA_REF_METHOD_H_

#include "lua/callback.h"

namespace lua {

namespace internal {

// Helper class to store information about the method.
template<typename T>
struct RefMethodRef {
  RefMethodRef(T func, int ref_arg, bool ref)
      : func(func), ref_arg(ref_arg), ref(ref) {}

  T func;
  int ref_arg;
  bool ref;
};

// Store the |arg| into the object at index |1|.
void StoreArg(State* state, int arg, bool ref);

// Wrapper of the method to automatically store a reference of argument.
template<typename T>
int RefMethodWrapper(State* state) {
  // DO NOT USE ANY C++ STACK.
  RefMethodRef<T>* method = static_cast<RefMethodRef<T>*>(
      lua_touserdata(state, lua_upvalueindex(1)));
  // Add reference to the argument.
  StoreArg(state, method->ref_arg + 1, method->ref);
  // Call the method.
  using ReturnType = typename FunctorTraits<T>::ReturnType;
  int nargs = GetTop(state);
  int nresults = Values<ReturnType>::count;
  Push(state, method->func);
  lua_insert(state, 1);
  lua_call(state, nargs, nresults);
  return nresults;
}

}  // namespace internal

// Used for binding member functions when we want to store a reference to the
// argument in the object.
// This helper function is used for automatically deducing T.
template<typename T>
internal::RefMethodRef<T> RefMethod(T func, int ref_arg, bool ref) {
  return internal::RefMethodRef<T>(func, ref_arg, ref);
}

// Converter for RefMethodRef.
template<typename T>
struct Type<internal::RefMethodRef<T>> {
  static constexpr const char* name = "method";
  static inline void Push(State* state,
                          const internal::RefMethodRef<T>& method) {
    NewUserData<internal::RefMethodRef<T>>(state, method);
    lua_pushcclosure(state, &internal::RefMethodWrapper<T>, 1);
  }
};

}  // namespace lua

#endif  // LUA_REF_METHOD_H_
