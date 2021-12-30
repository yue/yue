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

  v8::Local<v8::Object> GetHandle() const;
  v8::Isolate* GetIsolate() const;

 private:
  static void WeakCallback(const v8::WeakCallbackInfo<ObjectTracker>& data);

  v8::Isolate* isolate_;
  v8::Global<v8::Object> v8_ref_;
};

// Tracks the lifetime of v8::Object that wraps RefPtr.
template<typename T>
class RefPtrObjectTracker : public ObjectTracker {
 public:
  RefPtrObjectTracker(v8::Isolate* isolate, v8::Local<v8::Object> obj, T* ptr)
      : ObjectTracker(isolate, obj), ptr_(ptr) {
    obj->SetAlignedPointerInInternalField(0, ptr);
    ptr_->AddRef();
    PerIsolateData::Get(isolate)->SetObjectTracker(ptr, this);
  }

  ~RefPtrObjectTracker() override {
    PerIsolateData::Get(GetIsolate())->SetObjectTracker(ptr_, nullptr);
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
    obj->SetAlignedPointerInInternalField(0, this);
  }

  T* Get() {
    return ptr_.get();
  }

 private:
  base::WeakPtr<T> ptr_;
};


// Get or create FunctionTemplate, returns true if the |name| exists.
bool GetOrCreateFunctionTemplate(
    v8::Isolate* isolate,
    void* key,
    v8::Local<v8::FunctionTemplate>* templ);

// Get populated prototype for T.
template<typename T>
bool GetOrCreatePrototype(
    v8::Local<v8::Context> context,
    const char* name,
    v8::Local<v8::FunctionTemplate>* templ) {
  // Each type would get its own storage of |key| variable, and we use its
  // address as the key to the type's FunctionTemplate.
  static int key = 0xFEE;
  if (GetOrCreateFunctionTemplate(context->GetIsolate(), &key, templ))
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

}  // namespace internal

}  // namespace vb

#endif  // V8BINDING_PROTOTYPE_INTERNAL_H_
