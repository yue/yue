// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_REF_METHOD_H_
#define LUA_REF_METHOD_H_

#include <utility>

#include "lua/callback.h"

namespace lua {

enum class RefType {
  Ref,
  Deref,
  Reset,
};

namespace internal {

// Helper class to store information about the method.
template<typename T>
struct RefMethodArgs {
  RefMethodArgs(T func, int ref_arg, RefType ref_type, const char* ref_key)
      : func(func), ref_arg(ref_arg), ref_type(ref_type), ref_key(ref_key) {}

  T func;
  int ref_arg;
  RefType ref_type;
  const char* ref_key;
};

template<typename T>
struct RefMethodRef {
  RefMethodRef(State* state, RefMethodArgs<T> args)
      : args(std::move(args)), ref_(state, -1) {}

  RefMethodRef& operator=(const RefMethodRef&) = delete;
  RefMethodRef(const RefMethodRef&) = delete;

  RefMethodArgs<T> args;

 private:
  // For Lua 5.1 on Windows, it is possible for an upvalue to be garbage
  // collected before the callback is called.
  Persistent ref_;
};

// Store the |arg| into the object at index |1|.
void StoreArg(State* state, int ref_arg, RefType ref_type, const char* ref_key);

// Wrapper of the method to automatically store a reference of argument.
template<typename T>
int RefMethodWrapper(State* state) {
  const RefMethodArgs<T>& args = static_cast<RefMethodRef<T>*>(
      lua_touserdata(state, lua_upvalueindex(1)))->args;
  // Add reference to the argument.
  StoreArg(state, args.ref_arg + 1, args.ref_type, args.ref_key);
  // Call the method.
  using ReturnType = typename FunctorTraits<T>::ReturnType;
  int nargs = GetTop(state);
  int nresults = Values<ReturnType>::count;
  Push(state, args.func);
  lua_insert(state, 1);
  lua_call(state, nargs, nresults);
  return nresults;
}

}  // namespace internal

// Used for binding member functions when we want to store a reference to the
// argument in the object.
// This helper function is used for automatically deducing T.
template<typename T>
internal::RefMethodArgs<T> RefMethod(
    State* state, T func, RefType ref_type, const char* ref_key = nullptr,
    int arg = 1) {
  CHECK((ref_type == RefType::Reset && ref_key != nullptr) ||
        (ref_type != RefType::Reset && ref_key == nullptr));
  return internal::RefMethodArgs<T>(func, arg, ref_type, ref_key);
}

// Converter for RefMethodRef.
template<typename T>
struct Type<internal::RefMethodArgs<T>> {
  static constexpr const char* name = "method";
  static inline void Push(State* state, internal::RefMethodArgs<T> args) {
    NewUserData<internal::RefMethodRef<T>>(state, state, std::move(args));
    lua_pushcclosure(state, &internal::RefMethodWrapper<T>, 1);
  }
};

}  // namespace lua

#endif  // LUA_REF_METHOD_H_
