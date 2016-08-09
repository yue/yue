// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef LUA_CALLBACK_INTERNAL_H_
#define LUA_CALLBACK_INTERNAL_H_

#include "base/callback.h"
#include "lua/push.h"
#include "lua/to.h"

namespace lua {

namespace internal {

enum PushCFunctionFlags {
  HolderIsFirstArgument = 1 << 0,
};

template<typename T>
struct CallbackParamTraits {
  typedef T LocalType;
};
template<typename T>
struct CallbackParamTraits<const T&> {
  typedef T LocalType;
};
template<typename T>
struct CallbackParamTraits<const T*> {
  typedef T* LocalType;
};

template<typename T>
bool GetNextArgument(State* state, int index, int create_flags, T* result) {
  if (index == 0 && (create_flags & HolderIsFirstArgument) != 0) {
    // TODO(zcbenz): Add support for classes.
    return false;
  } else {
    // Lua uses 1-index array.
    return To(state, index + 1, result);;
  }
}

// Get how many values would be returned for the type.
// TODO(zcbenz): Support multiple arguments.
template<typename ReturnType>
struct ReturnValues {
  static const int count = 1;
};

template<>
struct ReturnValues<void> {
  static const int count = 0;
};

// CallbackHolder and CallbackHolderBase are used to pass a base::Callback from
// PushCFunction through DispatchToCallback, where it is invoked.

// This simple base class is used so that we can share a single object template
// among every CallbackHolder instance.
class CallbackHolderBase {
 protected:
  explicit CallbackHolderBase(State* state);

 private:
  static int OnGC(State* state);

  DISALLOW_COPY_AND_ASSIGN(CallbackHolderBase);
};

template<typename Sig>
class CallbackHolder : public CallbackHolderBase {
 public:
  CallbackHolder(State* state, const base::Callback<Sig>& callback, int flags)
      : CallbackHolderBase(state), callback(callback), flags(flags) {}

  base::Callback<Sig> callback;
  int flags;

 private:
  DISALLOW_COPY_AND_ASSIGN(CallbackHolder);
};

// Classes for generating and storing an argument pack of integer indices
// (based on well-known "indices trick", see: http://goo.gl/bKKojn):
template <size_t... indices>
struct IndicesHolder {};

template <size_t requested_index, size_t... indices>
struct IndicesGenerator {
  using type = typename IndicesGenerator<requested_index - 1,
                                         requested_index - 1,
                                         indices...>::type;
};
template <size_t... indices>
struct IndicesGenerator<0, indices...> {
  using type = IndicesHolder<indices...>;
};

// Class template for extracting and storing single argument for callback
// at position |index|.
template <size_t index, typename ArgType>
struct ArgumentHolder {
  using ArgLocalType = typename CallbackParamTraits<ArgType>::LocalType;

  ArgLocalType value;
  bool ok;

  ArgumentHolder(State* state, int create_flags)
      : ok(GetNextArgument(state, index, create_flags, &value)) {}
};

// Class template for converting arguments from JavaScript to C++ and running
// the callback with them.
template <typename IndicesType, typename... ArgTypes>
class Invoker {};

template <size_t... indices, typename... ArgTypes>
class Invoker<IndicesHolder<indices...>, ArgTypes...>
    : public ArgumentHolder<indices, ArgTypes>... {
 public:
  // Invoker<> inherits from ArgumentHolder<> for each argument.
  // C++ has always been strict about the class initialization order,
  // so it is guaranteed ArgumentHolders will be initialized (and thus, will
  // extract arguments from Arguments) in the right order.
  Invoker(State* state, int create_flags)
      : ArgumentHolder<indices, ArgTypes>(state, create_flags)...,
        state_(state) {}

  bool IsOK() {
    return And(ArgumentHolder<indices, ArgTypes>::ok...);
  }

  template<typename ReturnType>
  bool DispatchToCallback(base::Callback<ReturnType(ArgTypes...)> callback) {
    ReturnType ret = callback.Run(ArgumentHolder<indices, ArgTypes>::value...);
    return Push(state_, ret);
  }

  // In C++, you can declare the function foo(void), but you can't pass a void
  // expression to foo. As a result, we must specialize the case of Callbacks
  // that have the void return type.
  bool DispatchToCallback(base::Callback<void(ArgTypes...)> callback) {
    callback.Run(ArgumentHolder<indices, ArgTypes>::value...);
    return true;
  }

 private:
  static bool And() { return true; }
  template <typename... T>
  static bool And(bool arg1, T... args) {
    return arg1 && And(args...);
  }

  State* state_;
};

// DispatchToCallback converts all the lua arguments to C++ types and
// invokes the base::Callback.
template <typename Sig>
struct Dispatcher {};

template <typename ReturnType, typename... ArgTypes>
struct Dispatcher<ReturnType(ArgTypes...)> {
  static int DispatchToCallback(State* state) {
    typedef CallbackHolder<ReturnType(ArgTypes...)> HolderT;
    HolderT* holder = static_cast<HolderT*>(
        lua_touserdata(state, lua_upvalueindex(1)));

    bool ok = false;
    do {  // Make sure C++ stack is destroyed before calling lua_error.
      using Indices = typename IndicesGenerator<sizeof...(ArgTypes)>::type;
      Invoker<Indices, ArgTypes...> invoker(state, holder->flags);
      if (!invoker.IsOK()) {
        // TODO(zcbenz): Get the index of failing argument.
        Push(state, "Failed to convert arguments");
        break;
      }
      if (!invoker.DispatchToCallback(holder->callback)) {
        Push(state, "Failed to convert result");
        break;
      }
      ok = true;
    } while (false);

    if (!ok) {  // Throw error after we are out of C++ stack.
      lua_error(state);
      NOTREACHED() << "Code after lua_error() gets called";
      return -1;
    }

    return internal::ReturnValues<ReturnType>::count;
  }
};

// Push the function on stack without wrapping it with pcall.
template<typename Sig>
inline bool PushCFunction(State* state,
                          const base::Callback<Sig>& callback,
                          int callback_flags = 0) {
  typedef CallbackHolder<Sig> HolderT;
  void* holder = lua_newuserdata(state, sizeof(HolderT));
  new(holder) HolderT(state, callback, callback_flags);

  lua_pushcclosure(state, &internal::Dispatcher<Sig>::DispatchToCallback, 1);
  return true;  // ignore memory errors.
}

}  // namespace internal

}  // namespace lua

#endif  // LUA_CALLBACK_INTERNAL_H_
