// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_UTIL_FUNCTION_CALLER_H_
#define NATIVEUI_UTIL_FUNCTION_CALLER_H_

#include <string>
#include <utility>

#include "base/values.h"

namespace nu {

namespace internal {

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

// Converters for various types.
template<typename T>
inline void GetArgument(base::Value arg, T* value) {
  // Just use default value by default.
}

inline void GetArgument(base::Value arg, base::Value* value) {
  *value = std::move(arg);
}

inline void GetArgument(base::Value arg, base::ListValue* value) {
  if (arg.is_list())
    *value = std::move(*static_cast<base::ListValue*>(&arg));
}

inline void GetArgument(base::Value arg, base::DictionaryValue* value) {
  if (arg.is_dict())
    *value = std::move(*static_cast<base::DictionaryValue*>(&arg));
}

inline void GetArgument(base::Value arg, bool* value) {
  if (arg.is_bool())
    *value = arg.GetBool();
}

inline void GetArgument(base::Value arg, double* value) {
  if (arg.is_double())
    *value = arg.GetDouble();
}

inline void GetArgument(base::Value arg, float* value) {
  if (arg.is_double())
    *value = static_cast<float>(arg.GetDouble());
}

inline void GetArgument(base::Value arg, int* value) {
  if (arg.is_int())
    *value = arg.GetInt();
}

inline void GetArgument(base::Value arg, std::string* value) {
  if (arg.is_string())
    *value = arg.GetString();
}

// Class template for extracting and storing single argument for callback
// at position |index|.
template<size_t index, typename ArgType>
struct ArgumentHolder {
  using ArgLocalType = typename CallbackParamTraits<ArgType>::LocalType;

  ArgLocalType arg;

  explicit ArgumentHolder(base::Value* value) {
    if (value->GetList().size() > index)
      GetArgument(std::move(value->GetList()[index]), &arg);
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
  explicit Invoker(base::Value args)
      : ArgumentHolder<indices, ArgTypes>(&args)... {
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
                                 base::Value args) {
    DCHECK(args.is_list());
    using Indices = typename IndicesGenerator<sizeof...(ArgTypes)>::type;
    Invoker<Indices, ArgTypes...> invoker(std::move(args));
    invoker.DispatchToCallback(func);
  }
};

}  // namespace internal

}  // namespace nu

#endif  // NATIVEUI_UTIL_FUNCTION_CALLER_H_
