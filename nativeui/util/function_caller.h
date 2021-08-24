// Copyright 2018 Cheng Zhao. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_UTIL_FUNCTION_CALLER_H_
#define NATIVEUI_UTIL_FUNCTION_CALLER_H_

#include <string>
#include <type_traits>
#include <utility>

#include "base/values.h"

namespace nu {

class Browser;

namespace internal {

// Deduce the proper type for callback parameters.
template<typename T>
struct CallbackParamTraits {
  using LocalType = typename std::decay<T>::type;
};
template<typename T>
struct CallbackParamTraits<const T*> {
  using LocalType = T*;
};
template<>
struct CallbackParamTraits<const char*> {
  using LocalType = const char*;
};
template<>
struct CallbackParamTraits<const char*&> {
  using LocalType = const char*;
};

// Classes for generating and storing an argument pack of integer indices
// (based on well-known "indices trick", see: http://goo.gl/bKKojn):
template<size_t... indices>
struct IndicesHolder {};

template<size_t requested_index, size_t... indices>
struct IndicesGenerator {
  using type = typename IndicesGenerator<requested_index - 1,
                                         requested_index - 1,
                                         indices...>::type;
};
template<size_t... indices>
struct IndicesGenerator<0, indices...> {
  using type = IndicesHolder<indices...>;
};

// A struct holding current arg.
struct CallContext {
  nu::Browser* browser;
  size_t current_arg = 0;
};

// Converters for various types.
template<typename T>
inline void GetArgument(CallContext* context, base::Value* arg, T* value) {
  // Just use default value by default.
  context->current_arg++;
}

inline void GetArgument(CallContext* context, base::Value* arg,
                        Browser** value) {
  *value = context->browser;
}

inline void GetArgument(CallContext* context, base::Value* arg,
                        base::Value* value) {
  *value = std::move(*arg);
  context->current_arg++;
}

inline void GetArgument(CallContext* context, base::Value* arg,
                        std::string* value) {
  if (arg->is_string())
    *value = arg->GetString();
  context->current_arg++;
}

inline void GetArgument(CallContext* context, base::Value* arg, bool* value) {
  if (arg->is_bool())
    *value = arg->GetBool();
  context->current_arg++;
}

inline void GetArgument(CallContext* context, base::Value* arg, double* value) {
  if (arg->is_double())
    *value = arg->GetDouble();
  context->current_arg++;
}

inline void GetArgument(CallContext* context, base::Value* arg, float* value) {
  if (arg->is_double())
    *value = static_cast<float>(arg->GetDouble());
  context->current_arg++;
}

inline void GetArgument(CallContext* context, base::Value* arg, int* value) {
  if (arg->is_int())
    *value = arg->GetInt();
  context->current_arg++;
}

// Class template for extracting and storing single argument for callback
// at position |index|.
template<size_t index, typename ArgType>
struct ArgumentHolder {
  using ArgLocalType = typename CallbackParamTraits<ArgType>::LocalType;

  ArgLocalType arg;

  ArgumentHolder(CallContext* context, base::Value* value) {
    size_t current_arg = context->current_arg;
    if (current_arg < value->GetList().size())
      GetArgument(context, &value->GetList()[current_arg], &arg);
  }
};

// Class template for converting arguments from base::Value to C++ and running
// the callback with them.
template<typename IndicesType, typename... ArgTypes>
class Invoker {};

template<size_t... indices, typename... ArgTypes>
class Invoker<IndicesHolder<indices...>, ArgTypes...>
    : public ArgumentHolder<indices, ArgTypes>... {
 public:
  Invoker(CallContext* context, base::Value args)
      : ArgumentHolder<indices, ArgTypes>(context, &args)... {
  }

  void DispatchToCallback(const std::function<void(ArgTypes...)>& callback) {
    callback(std::move(ArgumentHolder<indices, ArgTypes>::arg)...);
  }
};

// DispatchToCallback converts all the base::Value arguments to C++ types and
// invokes the std::function.
template<typename Sig>
struct Dispatcher {};

template<typename... ArgTypes>
struct Dispatcher<void(ArgTypes...)> {
  static void DispatchToCallback(const std::function<void(ArgTypes...)>& func,
                                 Browser* browser,
                                 base::Value args) {
    DCHECK(args.is_list());
    CallContext context = { browser, 0 };
    using Indices = typename IndicesGenerator<sizeof...(ArgTypes)>::type;
    Invoker<Indices, ArgTypes...> invoker(&context, std::move(args));
    invoker.DispatchToCallback(func);
  }
};

// Evaluated to RunType of the given callable type.
// Example:
//   auto f = [](int, char*) { return 0.1; };
//   ExtractCallableRunType<decltype(f)>
//   is evaluated to
//   double(int, char*);
template <typename Callable,
          typename Signature = decltype(&Callable::operator())>
struct ExtractCallableRunTypeImpl;

template <typename Callable, typename R, typename... Args>
struct ExtractCallableRunTypeImpl<Callable, R(Callable::*)(Args...) const> {
  using Type = R(Args...);
};

template <typename Callable>
using ExtractCallableRunType =
    typename ExtractCallableRunTypeImpl<Callable>::Type;

// IsConvertibleToRunType<Functor> is std::true_type if |Functor| has operator()
// and convertible to the corresponding function pointer. Otherwise, it's
// std::false_type.
// Example:
//   IsConvertibleToRunType<void(*)()>::value is false.
//
//   struct Foo {};
//   IsConvertibleToRunType<void(Foo::*)()>::value is false.
//
//   auto f = []() {};
//   IsConvertibleToRunType<decltype(f)>::value is true.
//
//   int i = 0;
//   auto g = [i]() {};
//   IsConvertibleToRunType<decltype(g)>::value is false.
template <typename Functor, typename SFINAE = void>
struct IsConvertibleToRunType : std::false_type {};

template <typename Callable>
struct IsConvertibleToRunType<Callable,
                              base::void_t<decltype(&Callable::operator())>>
    : std::is_convertible<Callable, ExtractCallableRunType<Callable>*> {};

// FunctorTraits<>
// Type traits used to determine the correct RunType and invocation manner for
// a Functor. This is where function signature adapters are applied.
template <typename Functor, typename SFINAE = void>
struct FunctorTraits;

// For a callable type that is convertible to the corresponding function type.
// This specialization is intended to allow binding captureless lambdas by
// base::Bind(), based on the fact that captureless lambdas can be convertible
// to the function type while capturing lambdas can't.
template <typename Functor>
struct FunctorTraits<
    Functor,
    typename std::enable_if<IsConvertibleToRunType<Functor>::value>::type> {
  using RunType = ExtractCallableRunType<Functor>;
  static constexpr bool is_method = false;
  static constexpr bool is_nullable = false;
};

// For functions.
template <typename R, typename... Args>
struct FunctorTraits<R (*)(Args...)> {
  using RunType = R(Args...);
  static constexpr bool is_method = false;
  static constexpr bool is_nullable = true;

  template <typename... RunArgs>
  static R Invoke(R (*function)(Args...), RunArgs&&... args) {
    return function(std::forward<RunArgs>(args)...);
  }
};

#if defined(OS_WIN) && !defined(ARCH_CPU_64_BITS)

// For functions.
template <typename R, typename... Args>
struct FunctorTraits<R(__stdcall*)(Args...)> {
  using RunType = R(Args...);
  static constexpr bool is_method = false;
  static constexpr bool is_nullable = true;

  template <typename... RunArgs>
  static R Invoke(R(__stdcall* function)(Args...), RunArgs&&... args) {
    return function(std::forward<RunArgs>(args)...);
  }
};

// For functions.
template <typename R, typename... Args>
struct FunctorTraits<R(__fastcall*)(Args...)> {
  using RunType = R(Args...);
  static constexpr bool is_method = false;
  static constexpr bool is_nullable = true;

  template <typename... RunArgs>
  static R Invoke(R(__fastcall* function)(Args...), RunArgs&&... args) {
    return function(std::forward<RunArgs>(args)...);
  }
};

#endif  // defined(OS_WIN) && !defined(ARCH_CPU_64_BITS)

// For methods.
template <typename R, typename Receiver, typename... Args>
struct FunctorTraits<R (Receiver::*)(Args...)> {
  using RunType = R(Receiver*, Args...);
  static constexpr bool is_method = true;
  static constexpr bool is_nullable = true;

  template <typename ReceiverPtr, typename... RunArgs>
  static R Invoke(R (Receiver::*method)(Args...),
                  ReceiverPtr&& receiver_ptr,
                  RunArgs&&... args) {
    return ((*receiver_ptr).*method)(std::forward<RunArgs>(args)...);
  }
};

// For const methods.
template <typename R, typename Receiver, typename... Args>
struct FunctorTraits<R (Receiver::*)(Args...) const> {
  using RunType = R(const Receiver*, Args...);
  static constexpr bool is_method = true;
  static constexpr bool is_nullable = true;

  template <typename ReceiverPtr, typename... RunArgs>
  static R Invoke(R (Receiver::*method)(Args...) const,
                  ReceiverPtr&& receiver_ptr,
                  RunArgs&&... args) {
    return ((*receiver_ptr).*method)(std::forward<RunArgs>(args)...);
  }
};

}  // namespace internal

}  // namespace nu

#endif  // NATIVEUI_UTIL_FUNCTION_CALLER_H_
