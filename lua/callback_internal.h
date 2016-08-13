// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_CALLBACK_INTERNAL_H_
#define LUA_CALLBACK_INTERNAL_H_

#include "base/callback.h"
#include "lua/call_context.h"
#include "lua/handle.h"
#include "lua/pcall.h"

namespace lua {

namespace internal {

// Deduce the proper type for callback parameters.
template<typename T>
struct CallbackParamTraits {
  typedef T LocalType;
};
template<typename T>
struct CallbackParamTraits<const T&> {
  typedef T LocalType;
};
template<typename T>
struct CallbackParamTraits<T*> {
  typedef T* LocalType;
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
  CallbackHolder(State* state, const base::Callback<Sig>& callback)
      : CallbackHolderBase(state), callback(callback) {}

  base::Callback<Sig> callback;

 private:
  DISALLOW_COPY_AND_ASSIGN(CallbackHolder);
};

// Class template for extracting and storing single argument for callback
// at position |index|.
template<size_t index, typename ArgType>
struct ArgumentHolder {
  using ArgLocalType = typename CallbackParamTraits<ArgType>::LocalType;

  ArgLocalType value;
  const bool ok;

  explicit ArgumentHolder(CallContext* context)
      : ok(To(context->state, context->current_arg, &value)) {
    if (ok) {
      context->current_arg++;
    } else {
      context->invalid_arg = context->current_arg;
      context->invalid_arg_name = Type<ArgLocalType>::name;
    }
  }
};

// The holder for State*.
template<size_t index>
struct ArgumentHolder<index, State*> {
  State* value;
  const bool ok = true;
  explicit ArgumentHolder(CallContext* context) : value(context->state) {}
};

// The holder for CallContext*
template<size_t index>
struct ArgumentHolder<index, CallContext*> {
  CallContext* value;
  const bool ok = true;
  explicit ArgumentHolder(CallContext* context) : value(context) {}
};

// Class template for converting arguments from JavaScript to C++ and running
// the callback with them.
template<typename IndicesType, typename... ArgTypes>
class Invoker {};

template<size_t... indices, typename... ArgTypes>
class Invoker<IndicesHolder<indices...>, ArgTypes...>
    : public ArgumentHolder<indices, ArgTypes>... {
 public:
  // Invoker<> inherits from ArgumentHolder<> for each argument.
  // C++ has always been strict about the class initialization order,
  // so it is guaranteed ArgumentHolders will be initialized (and thus, will
  // extract arguments from Arguments) in the right order.
  explicit Invoker(CallContext* context)
      : ArgumentHolder<indices, ArgTypes>(context)...,
        context_(context) {}

  bool IsOK() {
    return And(ArgumentHolder<indices, ArgTypes>::ok...);
  }

  template<typename ReturnType>
  void DispatchToCallback(
      const base::Callback<ReturnType(ArgTypes...)>& callback) {
    Push(context_->state,
         callback.Run(ArgumentHolder<indices, ArgTypes>::value...));
  }

  // In C++, you can declare the function foo(void), but you can't pass a void
  // expression to foo. As a result, we must specialize the case of Callbacks
  // that have the void return type.
  void DispatchToCallback(const base::Callback<void(ArgTypes...)>& callback) {
    callback.Run(ArgumentHolder<indices, ArgTypes>::value...);
  }

 private:
  static bool And() { return true; }
  template<typename... T>
  static bool And(bool arg1, T... args) {
    return arg1 && And(args...);
  }

  CallContext* context_;
};

// DispatchToCallback converts all the lua arguments to C++ types and
// invokes the base::Callback.
template<typename Sig>
struct Dispatcher {};

template<typename ReturnType, typename... ArgTypes>
struct Dispatcher<ReturnType(ArgTypes...)> {
  static int DispatchToCallback(State* state) {
    // Receive the callback from userdata.
    typedef CallbackHolder<ReturnType(ArgTypes...)> HolderT;
    HolderT* holder = static_cast<HolderT*>(
        lua_touserdata(state, lua_upvalueindex(1)));

    CallContext context(state);
    {  // Make sure C++ stack is destroyed before calling lua_error.
      using Indices = typename IndicesGenerator<sizeof...(ArgTypes)>::type;
      Invoker<Indices, ArgTypes...> invoker(&context);
      if (!invoker.IsOK()) {
        context.has_error = true;
        if (GetType(state, context.invalid_arg) == LuaType::None) {
          PushFormatedString(state, "insufficient args, only %d supplied",
                             context.invalid_arg -1);
        } else {
          PushFormatedString(
              state, "error converting arg at index %d from %s to %s",
              context.invalid_arg,
              lua_typename(state, lua_type(state, context.invalid_arg)),
              context.invalid_arg_name);
        }
      } else {
        invoker.DispatchToCallback(holder->callback);
      }
    }

    if (context.has_error) {  // Throw error after we are out of C++ stack.
      lua_error(state);
      NOTREACHED() << "Code after lua_error() gets called";
      return -1;
    }

    return Values<ReturnType>::count;
  }
};

// Push the function on stack without wrapping it with pcall.
template<typename Sig>
inline void PushCFunction(State* state, const base::Callback<Sig>& callback) {
  typedef CallbackHolder<Sig> HolderT;
  void* holder = lua_newuserdata(state, sizeof(HolderT));
  new(holder) HolderT(state, callback);

  lua_pushcclosure(state, &internal::Dispatcher<Sig>::DispatchToCallback, 1);
}

// Call PCall for the gloal handle.
template<typename ReturnType, typename...ArgTypes>
struct PCallHelper {
  static ReturnType Run(State* state, const std::unique_ptr<Persistent>& handle,
                        ArgTypes... args) {
    ReturnType result = ReturnType();
    int top = GetTop(state);
    handle->Push(state);
    PCall(state, &result, args...);
    SetTop(state, top);  // reset everything on stack
    return result;
  }
};

// The void return type version for PCallHelper.
template<typename...ArgTypes>
struct PCallHelper<void, ArgTypes...> {
  static void Run(State* state, const std::unique_ptr<Persistent>& handle,
                  ArgTypes... args) {
    int top = GetTop(state);
    handle->Push(state);
    PCall(state, nullptr, args...);
    SetTop(state, top);  // reset everything on stack
  }
};

}  // namespace internal

}  // namespace lua

#endif  // LUA_CALLBACK_INTERNAL_H_
