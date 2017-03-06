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
  // Get the constructor of the prototype.
  static inline v8::Local<v8::Function> Get(v8::Local<v8::Context> context) {
    return internal::GetConstructor<T>(context);
  }

  // Create an instance of T and store it in an v8::Object.
  template<typename... ArgTypes>
  static v8::Local<v8::Value> NewInstance(v8::Local<v8::Context> context,
                                          const ArgTypes&... args) {
    // Pass an External to indicate it is called from native code.
    v8::Local<v8::Function> constructor = Get(context);
    v8::Local<v8::Value> indicator = v8::External::New(
        context->GetIsolate(), nullptr);
    v8::MaybeLocal<v8::Object> result = constructor->NewInstance(
        context, 1, &indicator);
    if (result.IsEmpty())
      return v8::Null(context->GetIsolate());
    // Store the new instance in the object.
    v8::Local<v8::Object> obj = result.ToLocalChecked();
    T* instance = new T(args...);
    obj->SetAlignedPointerInInternalField(0, instance);
    // Keep track of its lifetime.
    new internal::RefPtrObjectTracker<T>(context->GetIsolate(), obj, instance);
    return obj;
  }
};

// The default type information for RefCounted class.
template<typename T>
struct Type<T*, typename std::enable_if<std::is_base_of<
                    base::subtle::RefCountedBase, T>::value>::type> {
  static constexpr const char* name = Type<T>::name;
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     T** out) {
    // Verify the type.
    if (!value->IsObject())
      return false;
    v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(value);
    if (obj->InternalFieldCount() != 1)
      return false;
    // Convert pointer to actual class.
    *out = static_cast<T*>(obj->GetAlignedPointerFromInternalField(0));
    return true;
  }
};

// Create prototype for classes that produce WeakPtr.
template<typename T>
struct Prototype<T, typename std::enable_if<std::is_base_of<
                        base::internal::WeakPtrBase,
                        decltype(((T*)nullptr)->GetWeakPtr())>::value>::type> {
  // Get the constructor of the prototype.
  static inline v8::Local<v8::Function> Get(v8::Local<v8::Context> context) {
    return internal::GetConstructor<T>(context);
  }
};

}  // namespace vb

#endif  // V8BINDING_PROTOTYPE_H_
