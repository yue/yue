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
      ToV8<v8::Local<v8::External>>(context, holder->GetHandle(isolate)));
  tmpl->RemovePrototype();
  return tmpl;
}

// Specialize for native functions.
template<typename T>
struct Type<T, typename std::enable_if<
                   internal::is_function_pointer<T>::value>::type> {
  static constexpr const char* name = "function";
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          T callback) {
    return CreateFunctionTemplate(context, base::Bind(callback))->GetFunction();
  }
  static inline v8::Local<v8::Data> ToV8Data(v8::Local<v8::Context> context,
                                             T callback) {
    return CreateFunctionTemplate(context, base::Bind(callback));
  }
};

// Specialize for member function.
template<typename T>
struct Type<T, typename std::enable_if<
                   std::is_member_function_pointer<T>::value>::type> {
  static constexpr const char* name = "method";
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          T callback) {
    return CreateFunctionTemplate(context, base::Bind(callback),
                                  HolderIsFirstArgument)->GetFunction();
  }
  static inline v8::Local<v8::Data> ToV8Data(v8::Local<v8::Context> context,
                                             T callback) {
    return CreateFunctionTemplate(context, base::Bind(callback));
  }
};

}  // namespace vb

#endif  // V8BINDING_CALLBACK_H_
