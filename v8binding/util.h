// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_UTIL_H_
#define V8BINDING_UTIL_H_

namespace vb {

template<typename T, typename... ArgTypes>
T* CreateOnHeap(ArgTypes... args) {
  return new T(args...);
}

template<typename T, typename... ArgTypes>
T CreateOnStack(ArgTypes... args) {
  return T(args...);
}

}  // namespace vb

#endif  // V8BINDING_UTIL_H_
