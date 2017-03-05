// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_PROTOTYPE_INTERNAL_H_
#define V8BINDING_PROTOTYPE_INTERNAL_H_

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "v8binding/types.h"
#include "v8binding/per_isolate_data.h"

namespace vb {

namespace internal {

// Get or create FunctionTemplate, returns true if the |name| exists.
bool GetOrCreateFunctionTemplate(
    v8::Isolate* isolate,
    const char* name,
    v8::Local<v8::FunctionTemplate>* templ);

// Create prototype inheritance chain for T and its BaseTypes.
template<typename T, typename Enable = void>
struct InheritanceChain {
  // There is no base type.
  static inline v8::Local<v8::FunctionTemplate> Get(v8::Isolate* isolate) {
    v8::Local<v8::FunctionTemplate> templ;
    GetOrCreateFunctionTemplate(isolate, Type<T>::name, &templ);
    return templ;
  }
};

template<typename T>
struct InheritanceChain<T, typename std::enable_if<std::is_class<
                               typename Type<T>::base>::value>::type> {
  static inline v8::Local<v8::FunctionTemplate> Get(v8::Isolate* isolate) {
    v8::Local<v8::FunctionTemplate> templ;
    if (GetOrCreateFunctionTemplate(isolate, Type<T>::name, &templ))
      return templ;

    // Inherit from base type's metatable.
    auto parent = InheritanceChain<typename Type<T>::base>::Get(isolate);
    templ->Inherit(parent);
    return templ;
  }
};

}  // namespace internal

}  // namespace vb

#endif  // V8BINDING_PROTOTYPE_INTERNAL_H_
