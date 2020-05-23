// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_PROTOTYPE_H_
#define V8BINDING_PROTOTYPE_H_

#include <string>
#include <type_traits>

#include "v8binding/prototype_internal.h"

namespace vb {

// Create constructor for the prototype.
template<typename T>
struct Constructor {
};
template<typename T>
inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                 Constructor<T>) {
  v8::Isolate* isolate = context->GetIsolate();
  auto templ = internal::InheritanceChain<T>::Get(context);
  auto constructor = templ->GetFunction(context).ToLocalChecked();
  // Build the constructor if we did not do it before.
  auto indicator = v8::Private::ForApi(isolate, ToV8Symbol(context, "vbb"));
  if (!constructor->HasPrivate(context, indicator).ToChecked()) {
    constructor->SetPrivate(context, indicator, v8::True(isolate));
    Type<T>::BuildConstructor(context, constructor);
  }
  return constructor;
}

// Create a new instance of v8::Object from the prototype of T.
template<typename T>
v8::MaybeLocal<v8::Object> CallConstructor(v8::Local<v8::Context> context) {
  // Pass an External to indicate it is called from native code.
  v8::Local<v8::Value> indicator = v8::External::New(
      context->GetIsolate(), nullptr);
  return ToV8(context, Constructor<T>()).template As<v8::Function>()->
      NewInstance(context, 1, &indicator);
}

// Check if |obj|'s prototype is inherited from T's prototype.
template<typename T>
bool IsPrototypeInheritedFrom(v8::Local<v8::Context> context,
                              v8::Local<v8::Object> obj) {
  std::string constructor;
  if (!FromV8(context, obj->GetConstructorName(), &constructor))
    return false;
  if (constructor == "Object")
    return false;
  if (constructor == Type<T>::name)
    return true;
  return IsPrototypeInheritedFrom<T>(
      context, obj->GetPrototype().As<v8::Object>());
}

// The default type information for RefCounted class.
template<typename T>
struct Type<T*, typename std::enable_if<std::is_base_of<
                    base::subtle::RefCountedBase, T>::value>::type> {
  static constexpr const char* name = Type<T>::name;
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context, T* ptr) {
    v8::Isolate* isolate = context->GetIsolate();
    if (!ptr)
      return v8::Null(isolate);
    // Whether there is already a wrapper for |ptr|.
    auto wrapper = PerIsolateData::Get(isolate)->GetObjectTracker(ptr);
    if (wrapper)
      return wrapper->GetHandle();
    // If not create a new wrapper for it.
    auto result = CallConstructor<T>(context);
    if (result.IsEmpty())
      return v8::Null(isolate);
    v8::Local<v8::Object> obj = result.ToLocalChecked();
    new internal::RefPtrObjectTracker<T>(isolate, obj, ptr);
    return obj;
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     T** out) {
    // Verify the type.
    if (!value->IsObject())
      return false;
    v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(value);
    if (obj->InternalFieldCount() != 1)
      return false;
    // Verify prototype chain.
    if (!IsPrototypeInheritedFrom<T>(context, obj))
      return false;
    // Convert pointer to actual class.
    *out = static_cast<T*>(obj->GetAlignedPointerFromInternalField(0));
    return *out != nullptr;
  }
};

// Helper for scoped_refptr.
template<typename T>
struct Type<scoped_refptr<T>> {
  static constexpr const char* name = Type<T>::name;
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   scoped_refptr<T> ptr) {
    return vb::ToV8(context, ptr.get());
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     scoped_refptr<T>* out) {
    T* out_ptr;
    if (!vb::FromV8(context, value, &out_ptr))
      return false;
    *out = out_ptr;
    return true;
  }
};

// The default type information for WeakPtr class.
template<typename T>
struct Type<T*, typename std::enable_if<std::is_base_of<
                    base::internal::WeakPtrBase,
                    decltype(((T*)nullptr)->GetWeakPtr())>::value>::type> {  // NOLINT
  static constexpr const char* name = Type<T>::name;
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context, T* ptr) {
    v8::Isolate* isolate = context->GetIsolate();
    if (!ptr)
      return v8::Null(isolate);
    // Do not cache the pointer of WeakPtr, because the pointer may point to a
    // variable on stack, which can have same address with previous variable on
    // the stack.
    auto result = CallConstructor<T>(context);
    if (result.IsEmpty())
      return v8::Null(isolate);
    // Store the new instance in the object.
    v8::Local<v8::Object> obj = result.ToLocalChecked();
    new internal::WeakPtrObjectTracker<T>(isolate, obj, ptr->GetWeakPtr());
    return obj;
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     T** out) {
    // Verify the type.
    if (!value->IsObject())
      return false;
    v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(value);
    if (obj->InternalFieldCount() != 1)
      return false;
    // Verify prototype chain.
    if (!IsPrototypeInheritedFrom<T>(context, obj))
      return false;
    // Convert pointer to actual class.
    auto* ptr = static_cast<internal::WeakPtrObjectTracker<T>*>(
        obj->GetAlignedPointerFromInternalField(0));
    if (!ptr || !ptr->Get())
      return false;
    *out = ptr->Get();
    return true;
  }
};

}  // namespace vb

#endif  // V8BINDING_PROTOTYPE_H_
