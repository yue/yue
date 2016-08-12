// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_TEMPLATE_UTIL_H_
#define LUA_TEMPLATE_UTIL_H_

#include <type_traits>

namespace lua {

namespace internal {

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

}  // namespace internal

}  // namespace lua

#endif  // LUA_TEMPLATE_UTIL_H_
