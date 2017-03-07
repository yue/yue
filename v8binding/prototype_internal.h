// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_PROTOTYPE_INTERNAL_H_
#define V8BINDING_PROTOTYPE_INTERNAL_H_

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "v8binding/per_isolate_data.h"
#include "v8binding/types.h"

namespace vb {

namespace internal {

// Common base for tracking lifetime of v8::Object.
class ObjectTracker {
 public:
  ObjectTracker(v8::Isolate* isolate, v8::Local<v8::Object> obj);
  virtual ~ObjectTracker();

 private:
  static void FirstWeakCallback(
      const v8::WeakCallbackInfo<ObjectTracker>& data);
  static void SecondWeakCallback(
      const v8::WeakCallbackInfo<ObjectTracker>& data);

  v8::Global<v8::Object> v8_ref_;

  DISALLOW_COPY_AND_ASSIGN(ObjectTracker);
};

// Tracks the lifetime of v8::Object that wraps RefPtr.
template<typename T>
class RefPtrObjectTracker : public ObjectTracker {
 public:
  RefPtrObjectTracker(v8::Isolate* isolate, v8::Local<v8::Object> obj, T* ptr)
      : ObjectTracker(isolate, obj), ptr_(ptr) {
    ptr_->AddRef();
  }

  ~RefPtrObjectTracker() override {
    ptr_->Release();
  }

 private:
  T* ptr_;
};

// A wrapper of WeakPtr which is stored in v8::Object.
template<typename T>
class WeakPtrObjectTracker : public ObjectTracker {
 public:
  WeakPtrObjectTracker(v8::Isolate* isolate, v8::Local<v8::Object> obj,
                       base::WeakPtr<T> ptr)
      : ObjectTracker(isolate, obj), ptr_(ptr) {
  }

  T* Get() {
    return ptr_.Get();
  }

 private:
  base::WeakPtr<T> ptr_;
};


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
  (*templ)->SetClassName(ToV8(context, name).As<v8::String>());
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

// Create constructor for the prototype.
template<typename T>
v8::Local<v8::Function> GetConstructor(v8::Local<v8::Context> context) {
  v8::Isolate* isolate = context->GetIsolate();
  auto templ = InheritanceChain<T>::Get(context);
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
  v8::Local<v8::Function> constructor = GetConstructor<T>(context);
  v8::Local<v8::Value> indicator = v8::External::New(
      context->GetIsolate(), nullptr);
  return constructor->NewInstance(context, 1, &indicator);
}

}  // namespace internal

}  // namespace vb

#endif  // V8BINDING_PROTOTYPE_INTERNAL_H_
