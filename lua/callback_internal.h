// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_CALLBACK_INTERNAL_H_
#define LUA_CALLBACK_INTERNAL_H_

#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "base/logging.h"
#include "base/notreached.h"
#include "base/template_util.h"
#include "lua/call_context.h"
#include "lua/handle.h"
#include "lua/pcall.h"
#include "lua/table.h"
#include "lua/user_data.h"

namespace lua {

namespace internal {

// CallbackHolder is used to pass a std::function from PushCFunction through
// DispatchToCallback, where it is invoked.
template<typename Sig>
class CallbackHolder {
 public:
  CallbackHolder(State* state, std::function<Sig> callback)
      : callback(std::move(callback)), ref_(state, -1) {}

  CallbackHolder& operator=(const CallbackHolder&) = delete;
  CallbackHolder(const CallbackHolder&) = delete;

  std::function<Sig> callback;

 private:
  // For Lua 5.1 on Windows, it is possible for an upvalue to be garbage
  // collected before the callback is called.
  Persistent ref_;
};

// Class template for extracting and storing single argument for callback
// at position |index|.
template<size_t index, typename ArgType>
struct ArgumentHolder {
  using ArgLocalType = typename CallbackParamTraits<ArgType>::LocalType;

  ArgLocalType value;

  bool Convert(CallContext* context) {
    bool ok = To(context->state, context->current_arg, &value);
    if (ok) {
      context->current_arg++;
    } else {
      context->invalid_arg = context->current_arg;
      context->invalid_arg_name = Type<ArgLocalType>::name;
    }
    return ok;
  }
};

// The holder for State*.
template<size_t index>
struct ArgumentHolder<index, State*> {
  State* value;
  bool Convert(CallContext* context) {
    value = context->state;
    return true;
  }
};

// The holder for CallContext*
template<size_t index>
struct ArgumentHolder<index, CallContext*> {
  CallContext* value;
  bool Convert(CallContext* context) {
    value = context;
    return true;
  }
};

// Class template for converting arguments from JavaScript to C++ and running
// the callback with them.
template<typename IndicesType, typename... ArgTypes>
class Invoker {};

template<size_t... indices, typename... ArgTypes>
class Invoker<IndicesHolder<indices...>, ArgTypes...>
    : public ArgumentHolder<indices, ArgTypes>... {
 public:
  explicit Invoker(CallContext* context) : context_(context) {}

  bool ConvertArgs() {
    return ConvertRestArgs(
        context_, IndicesHolder<indices...>(), ArgsHolder<ArgTypes...>());
  }

  template<typename ReturnType>
  void DispatchToCallback(
      const std::function<ReturnType(ArgTypes...)>& callback) {
    ReturnType r =
        callback(std::move(ArgumentHolder<indices, ArgTypes>::value)...);
    // Convert result to lua if there is no error happened.
    if (!context_->has_error)
      Push(context_->state, std::move(r));
  }

  // In C++, you can declare the function foo(void), but you can't pass a void
  // expression to foo. As a result, we must specialize the case of Callbacks
  // that have the void return type.
  void DispatchToCallback(const std::function<void(ArgTypes...)>& callback) {
    callback(std::move(ArgumentHolder<indices, ArgTypes>::value)...);
  }

 private:
  // Recusrively call Convert for each argument.
  bool ConvertRestArgs(CallContext*, IndicesHolder<>, ArgsHolder<>) {
    return true;
  }
  template<size_t... all_indices, typename... AllArgTypes>
  bool ConvertRestArgs(CallContext* context,
                       IndicesHolder<all_indices...>,
                       ArgsHolder<AllArgTypes...>) {
    using IS = IndicesSplitter<all_indices...>;
    using AS = ArgsSplitter<AllArgTypes...>;
    return ArgumentHolder<IS::first, typename AS::first>::Convert(context) &&
           ConvertRestArgs(context, typename IS::rest(), typename AS::rest());
  }

  CallContext* context_;
};

// DispatchToCallback converts all the lua arguments to C++ types and
// invokes the std::function.
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
    context.return_values_count = Values<ReturnType>::count;
    static_assert(std::is_trivially_destructible<CallContext>::value,
                  "The CallContext must not invole C++ stack");
    {  // Make sure C++ stack is destroyed before calling lua_error.
      using Indices = typename IndicesGenerator<sizeof...(ArgTypes)>::type;
      Invoker<Indices, ArgTypes...> invoker(&context);
      if (!invoker.ConvertArgs()) {
        context.has_error = true;
        if (GetType(state, context.invalid_arg) == LuaType::None) {
          PushFormatedString(state, "insufficient args, only %d supplied",
                             context.invalid_arg -1);
        } else {
          // Get name of custom type.
          std::string name;
          if (GetType(state, context.invalid_arg) == LuaType::UserData &&
              GetMetaTable(state, context.invalid_arg)) {
            RawGetAndPop(state, -1, "__name", &name);
            PopAndIgnore(state, 1);
          }

          PushFormatedString(
              state, "error converting arg at index %d from %s to %s",
              context.invalid_arg,
              name.empty() ? GetTypeName(state, context.invalid_arg)
                           : name.data(),
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

    return context.return_values_count;
  }
};

// Push the function on stack without wrapping it with pcall.
template<typename Sig>
inline void PushCFunction(State* state, std::function<Sig> callback) {
  NewUserData<CallbackHolder<Sig>>(state, state, std::move(callback));
  lua_pushcclosure(state, &internal::Dispatcher<Sig>::DispatchToCallback, 1);
}

// Call PCall for the gloal handle.
template<typename ReturnType, typename...ArgTypes>
struct PCallHelper {
  static ReturnType Run(State* state, std::shared_ptr<Handle> handle,
                        ArgTypes&&... args) {
    ReturnType result = ReturnType();
    int top = GetTop(state);
    DCHECK_EQ(state, handle->state());
    handle->Push();
    if (!PCall(state, &result, std::forward<ArgTypes>(args)...)) {
      std::string error;
      lua::Pop(state, &error);
      LOG(ERROR) << "Error when calling lua function: " << error;
    }
    SetTop(state, top);  // reset everything on stack
    return result;
  }
};

// The void return type version for PCallHelper.
template<typename...ArgTypes>
struct PCallHelper<void, ArgTypes...> {
  static void Run(State* state, std::shared_ptr<Handle> handle,
                  ArgTypes&&... args) {
    int top = GetTop(state);
    DCHECK_EQ(state, handle->state());
    handle->Push();
    if (!PCall(state, nullptr, std::forward<ArgTypes>(args)...)) {
      std::string error;
      lua::Pop(state, &error);
      LOG(ERROR) << "Error when calling lua function: " << error;
    }
    SetTop(state, top);  // reset everything on stack
  }
};

}  // namespace internal

}  // namespace lua

#endif  // LUA_CALLBACK_INTERNAL_H_
