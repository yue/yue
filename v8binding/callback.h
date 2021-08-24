// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_CALLBACK_H_
#define V8BINDING_CALLBACK_H_

#include <memory>
#include <utility>

#include "v8binding/callback_internal.h"

namespace vb {

// CreateFunctionTemplate creates a v8::FunctionTemplate that will create
// JavaScript functions that execute a provided C++ function or std::function.
// JavaScript arguments are automatically converted via Type<T>, as is
// the return value of the C++ function, if any.
//
// NOTE: V8 caches FunctionTemplates for a lifetime of a web page for its own
// internal reasons, thus it is generally a good idea to cache the template
// returned by this function.  Otherwise, repeated method invocations from JS
// will create substantial memory leaks. See http://crbug.com/463487.
template<typename Sig>
v8::Local<v8::FunctionTemplate> CreateFunctionTemplate(
    v8::Local<v8::Context> context, std::function<Sig> callback,
    int callback_flags = 0) {
#ifndef NDEBUG
  internal::FunctionTemplateCreated();
#endif
  v8::Isolate* isolate = context->GetIsolate();
  typedef internal::CallbackHolder<Sig> HolderT;
  HolderT* holder = new HolderT(isolate, std::move(callback), callback_flags);

  v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New(
      isolate, &internal::Dispatcher<Sig>::DispatchToCallback,
      holder->GetHandle(isolate));
  tmpl->RemovePrototype();
  return tmpl;
}

// Get function from v8, but does not keep a reference of it.
template<typename ReturnType, typename... ArgTypes>
bool WeakFunctionFromV8(v8::Local<v8::Context> context,
                        v8::Local<v8::Value> val,
                        std::function<ReturnType(ArgTypes...)>* out) {
  if (val->IsNull()) {
    *out = nullptr;
    return true;
  }
  if (!val->IsFunction())
    return false;
  v8::Isolate* isolate = context->GetIsolate();
  auto wrapper = std::make_shared<internal::V8FunctionWrapper>(
      isolate, val.As<v8::Function>());
  wrapper->SetWeak();
  *out = [isolate, wrapper](ArgTypes&&... args) -> ReturnType {
    return internal::V8FunctionInvoker<ReturnType(ArgTypes...)>::Go(
        isolate, wrapper, std::forward<ArgTypes>(args)...);
  };
  return true;
}

// Define how callbacks are converted.
template<typename ReturnType, typename... ArgTypes>
struct Type<std::function<ReturnType(ArgTypes...)>> {
  static constexpr const char* name = "Function";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> val,
                     std::function<ReturnType(ArgTypes...)>* out) {
    if (val->IsNull()) {
      *out = nullptr;
      return true;
    }
    if (!val->IsFunction())
      return false;
    v8::Isolate* isolate = context->GetIsolate();
    auto wrapper = std::make_shared<internal::V8FunctionWrapper>(
        isolate, val.As<v8::Function>());
    *out = [isolate, wrapper](ArgTypes&&... args) -> ReturnType {
      return internal::V8FunctionInvoker<ReturnType(ArgTypes...)>::Go(
          isolate, wrapper, std::forward<ArgTypes>(args)...);
    };
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
    using RunType = typename internal::FunctorTraits<T>::RunType;
    return CreateFunctionTemplate(context, std::function<RunType>(callback))
               ->GetFunction(context).ToLocalChecked();
  }
};

// Specialize for member function.
template<typename T>
struct Type<T, typename std::enable_if<
                   std::is_member_function_pointer<T>::value>::type> {
  static constexpr const char* name = "Method";
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          T callback) {
    using RunType = typename internal::FunctorTraits<T>::RunType;
    return CreateFunctionTemplate(context, std::function<RunType>(callback),
                                  HolderIsFirstArgument)->GetFunction(context)
                                                        .ToLocalChecked();
  }
};

}  // namespace vb

#endif  // V8BINDING_CALLBACK_H_
