// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_TEMPLATE_UTIL_H_
#define V8BINDING_TEMPLATE_UTIL_H_

#include <type_traits>
#include <utility>

namespace vb {

namespace internal {

// A clone of C++17 std::void_t.
// Unlike the original version, we need |make_void| as a helper struct to avoid
// a C++14 defect.
// ref: http://en.cppreference.com/w/cpp/types/void_t
// ref: http://open-std.org/JTC1/SC22/WG21/docs/cwg_defects.html#1558
template <typename...>
struct make_void {
  using type = void;
};

template <typename... Ts>
using void_t = typename make_void<Ts...>::type;

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

// Type trait for detecting function pointer.
template<typename Fun>
struct is_function_pointer
    : std::integral_constant<
          bool,
          std::is_pointer<Fun>::value &&
          std::is_function<typename std::remove_pointer<Fun>::type>::value> {};

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
struct IsConvertibleToRunType<Callable, void_t<decltype(&Callable::operator())>>
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

}  // namespace vb

#endif  // V8BINDING_TEMPLATE_UTIL_H_
