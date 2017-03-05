// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_PROTOTYPE_H_
#define V8BINDING_PROTOTYPE_H_

#include <type_traits>

#include "v8binding/prototype_internal.h"

namespace vb {

// Generate prototype for native classes.
template<typename T, typename Enable = void>
struct Prototype;

// Create prototype for RefCounted classes.
template<typename T>
struct Prototype<T, typename std::enable_if<std::is_base_of<
                        base::subtle::RefCountedBase, T>::value>::type> {
  static v8::Local<v8::Function> Get(v8::Local<v8::Context> context) {
    auto templ = internal::InheritanceChain<T>::Get(context->GetIsolate());
    auto result = templ->GetFunction(context);
    if (result.IsEmpty())
      return v8::Local<v8::Function>();
    else
      return result.ToLocalChecked();
  }
};

}  // namespace vb

#endif  // V8BINDING_PROTOTYPE_H_
