// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_REF_METHOD_H_
#define V8BINDING_REF_METHOD_H_

#include <utility>
#include <vector>

#include "v8binding/callback.h"

namespace vb {

enum class RefType {
  Ref,
  Deref,
  Reset,
};

namespace internal {

// Helper class to store information about the method.
template<typename T>
struct RefMethodRef {
  RefMethodRef(T func, int ref_arg, RefType ref_type, const char* ref_key)
      : func(func), ref_arg(ref_arg), ref_type(ref_type), ref_key(ref_key) {}

  T func;
  int ref_arg;
  RefType ref_type;
  const char* ref_key;
};

// Helper class to pass the RefMethodRef to v8 binding.
template<typename T>
class RefMethodRefHolder : public CallbackHolderBase {
 public:
  RefMethodRefHolder(v8::Isolate* isolate, RefMethodRef<T> ref)
      : CallbackHolderBase(isolate), ref(std::move(ref)) {}

  RefMethodRef<T> ref;
  v8::Global<v8::Function> v8_func;

 private:
  virtual ~RefMethodRefHolder() {}
};

// Store the |arg| into the |object|.
void StoreArg(v8::Local<v8::Context> context,
              v8::Local<v8::Object> object,
              v8::Local<v8::Value> ref_arg,
              RefType ref_type,
              const char* ref_key);

// Wrapper of the method to automatically store a reference of argument.
template<typename T>
void RefMethodWrapper(const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
  // Receive the callback information from v8.
  v8::Local<v8::External> v8_holder =
      v8::Local<v8::External>::Cast(info.Data());
  auto* holder = static_cast<RefMethodRefHolder<T>*>(v8_holder->Value());
  // Add reference to the argument.
  StoreArg(context,
           info.Holder(),
           info[holder->ref.ref_arg],
           holder->ref.ref_type,
           holder->ref.ref_key);
  // Create v8 function from the native function. Note that we need to cache
  // the function otherwise we would have a leak of FunctionTemplate.
  if (holder->v8_func.IsEmpty()) {
    auto func = v8::Local<v8::Function>::Cast(ToV8(context, holder->ref.func));
    holder->v8_func = v8::Global<v8::Function>(info.GetIsolate(), func);
  }
  // Call the original callback.
  v8::Local<v8::Function> callback =
      v8::Local<v8::Function>::New(info.GetIsolate(), holder->v8_func);
  std::vector<v8::Local<v8::Value>> args;
  args.reserve(info.Length());
  for (int i = 0; i < info.Length(); ++i)
    args.push_back(info[i]);
  callback->Call(context, info.This(), static_cast<int>(args.size()),
                 args.empty() ? nullptr : &args.front()).IsEmpty();
}

}  // namespace internal

// Used for binding member functions when we want to store a reference to the
// argument in the object.
// This helper function is used for automatically deducing T.
template<typename T>
internal::RefMethodRef<T> RefMethod(
    T func, RefType ref_type, const char* ref_key = nullptr, int arg = 0) {
  CHECK((ref_type == RefType::Reset && ref_key != nullptr) ||
        (ref_type != RefType::Reset && ref_key == nullptr));
  return internal::RefMethodRef<T>(func, arg, ref_type, ref_key);
}

}  // namespace vb

#endif  // V8BINDING_REF_METHOD_H_
