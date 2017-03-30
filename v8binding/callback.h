// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_CALLBACK_H_
#define V8BINDING_CALLBACK_H_

#include "v8binding/callback_internal.h"

namespace vb {

// CreateFunctionTemplate creates a v8::FunctionTemplate that will create
// JavaScript functions that execute a provided C++ function or base::Callback.
// JavaScript arguments are automatically converted via Type<T>, as is
// the return value of the C++ function, if any.
//
// NOTE: V8 caches FunctionTemplates for a lifetime of a web page for its own
// internal reasons, thus it is generally a good idea to cache the template
// returned by this function.  Otherwise, repeated method invocations from JS
// will create substantial memory leaks. See http://crbug.com/463487.
template<typename Sig>
v8::Local<v8::FunctionTemplate> CreateFunctionTemplate(
    v8::Local<v8::Context> context, const base::Callback<Sig> callback,
    int callback_flags = 0) {
  v8::Isolate* isolate = context->GetIsolate();
  typedef internal::CallbackHolder<Sig> HolderT;
  HolderT* holder = new HolderT(isolate, callback, callback_flags);

  v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New(
      isolate, &internal::Dispatcher<Sig>::DispatchToCallback,
      holder->GetHandle(isolate));
  tmpl->RemovePrototype();
  return tmpl;
}

// Define how callbacks are converted.
template<typename Sig>
struct Type<base::Callback<Sig>> {
  static constexpr const char* name = "Function";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> val,
                     base::Callback<Sig>* out) {
    if (val->IsNull()) {
      *out = base::Callback<Sig>();
      return true;
    }
    if (!val->IsFunction())
      return false;
    v8::Isolate* isolate = context->GetIsolate();
    *out = base::Bind(
        &internal::V8FunctionInvoker<Sig>::Go,
        isolate,
        base::RetainedRef(
            new internal::V8FunctionWrapper(isolate, val.As<v8::Function>())));
    return true;
  }
};

// Specialize for native functions.
template<typename T>
struct Type<T, typename std::enable_if<
                   internal::is_function_pointer<T>::value>::type> {
  static constexpr const char* name = "Function";
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          T callback) {
    return CreateFunctionTemplate(context, base::Bind(callback))->GetFunction();
  }
};

// Specialize for member function.
template<typename T>
struct Type<T, typename std::enable_if<
                   std::is_member_function_pointer<T>::value>::type> {
  static constexpr const char* name = "Method";
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          T callback) {
    return CreateFunctionTemplate(context, base::Bind(callback),
                                  HolderIsFirstArgument)->GetFunction();
  }
};

// Helper to push a type directly by invoking its constructor.
template<typename T, typename... ArgTypes>
struct NativeConstructor {
  static T Call(ArgTypes... args) {
    return std::move(T(args...));
  }
};
template<typename T, typename... ArgTypes>
inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                 NativeConstructor<T, ArgTypes...>) {
  auto constructor = base::Bind(&NativeConstructor<T, ArgTypes...>::Call);
  return CreateFunctionTemplate(context, constructor)->GetFunction();
}

}  // namespace vb

#endif  // V8BINDING_CALLBACK_H_
