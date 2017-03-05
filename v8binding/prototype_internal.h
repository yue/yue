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

// Get populated prototype for T.
template<typename T>
bool GetOrCreatePrototype(
    v8::Local<v8::Context> context,
    const char* name,
    v8::Local<v8::FunctionTemplate>* templ) {
  if (GetOrCreateFunctionTemplate(context->GetIsolate(), name, templ))
    return true;
  (*templ)->InstanceTemplate()->SetInternalFieldCount(1);
  Type<T>::BuildPrototype(context, (*templ)->PrototypeTemplate());
  return false;
}

// Create prototype inheritance chain for T and its BaseTypes.
template<typename T, typename Enable = void>
struct InheritanceChain {
  // There is no base type.
  static v8::Local<v8::FunctionTemplate> Get(v8::Local<v8::Context> context) {
    v8::Local<v8::FunctionTemplate> templ;
    GetOrCreatePrototype<T>(context, Type<T>::name, &templ);
    return templ;
  }
};

template<typename T>
struct InheritanceChain<T, typename std::enable_if<std::is_class<
                               typename Type<T>::base>::value>::type> {
  static v8::Local<v8::FunctionTemplate> Get(v8::Local<v8::Context> context) {
    v8::Local<v8::FunctionTemplate> templ;
    if (GetOrCreatePrototype<T>(context, Type<T>::name, &templ))
      return templ;

    // Inherit from base type's metatable.
    auto parent = InheritanceChain<typename Type<T>::base>::Get(context);
    templ->Inherit(parent);
    return templ;
  }
};

}  // namespace internal

}  // namespace vb

#endif  // V8BINDING_PROTOTYPE_INTERNAL_H_
