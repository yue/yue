// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_DICT_H_
#define V8BINDING_DICT_H_

#include "v8binding/ref_method.h"

namespace vb {

namespace internal {

// Define how can a type be converted to a field of ObjectTemplate.
template<typename T, typename Enable = void>
struct ToV8Data {
  static inline v8::Local<v8::Data> Do(v8::Local<v8::Context> context,
                                       const T& value) {
    return ToV8(context, value);
  }
};

// Specialize for native functions.
template<typename T>
struct ToV8Data<T, typename std::enable_if<
                       internal::is_function_pointer<T>::value>::type> {
  static inline v8::Local<v8::Data> Do(v8::Local<v8::Context> context,
                                       T callback) {
    using RunType = typename FunctorTraits<T>::RunType;
    return CreateFunctionTemplate(context, std::function<RunType>(callback));
  }
};

// Specialize for member function.
template<typename T>
struct ToV8Data<T, typename std::enable_if<
                       std::is_member_function_pointer<T>::value>::type> {
  static inline v8::Local<v8::Data> Do(v8::Local<v8::Context> context,
                                       T callback) {
    using RunType = typename FunctorTraits<T>::RunType;
    return CreateFunctionTemplate(context, std::function<RunType>(callback),
                                  HolderIsFirstArgument);
  }
};

// Specialize for ref method.
template<typename T>
struct ToV8Data<RefMethodRef<T>,
                typename std::enable_if<
                    std::is_member_function_pointer<T>::value>::type> {
  static inline v8::Local<v8::Data> Do(v8::Local<v8::Context> context,
                                       RefMethodRef<T> callback) {
    v8::Isolate* isolate = context->GetIsolate();
    auto* holder = new internal::RefMethodRefHolder<T>(isolate, callback);
    v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New(
        isolate, &internal::RefMethodWrapper<T>, holder->GetHandle(isolate));
    tmpl->RemovePrototype();
    return tmpl;
  }
};

}  // namespace internal

// Helper for setting Object.
template<typename Key, typename Value>
inline bool Set(v8::Local<v8::Context> context, v8::Local<v8::Object> object,
                const Key& key, const Value& value) {
  auto result = object->Set(context, ToV8(context, key), ToV8(context, value));
  return !result.IsNothing() && result.FromJust();
}

// Helper for setting ObjectTemplate.
template<typename Key, typename Value>
inline bool Set(v8::Local<v8::Context> context,
                v8::Local<v8::ObjectTemplate> templ,
                const Key& key, const Value& value) {
  templ->Set(ToV8(context, key).template As<v8::String>(),
             internal::ToV8Data<Value>::Do(context, value));
  return true;
}

// Allow setting arbitrary key/value pairs.
template<typename Dict, typename Key, typename Value, typename... ArgTypes>
inline bool Set(v8::Local<v8::Context> context, Dict dict,
                const Key& key, const Value& value,
                const ArgTypes&... args) {
  return Set(context, dict, key, value) && Set(context, dict, args...);
}

// Helper for getting from Object.
template<typename Key, typename Value>
inline bool Get(v8::Local<v8::Context> context, v8::Local<v8::Object> object,
                const Key& key, Value* out) {
  v8::Local<v8::Value> value;
  if (!object->Get(context, ToV8(context, key)).ToLocal(&value))
    return false;
  return FromV8(context, value, out);
}

// Allow getting arbitrary values.
template<typename Key, typename Value, typename... ArgTypes>
inline bool Get(v8::Local<v8::Context> context, v8::Local<v8::Object> object,
                const Key& key, Value* out, const ArgTypes&... args) {
  return Get(context, object, key, out) &&
         Get(context, object, args...);
}

}  // namespace vb

#endif  // V8BINDING_DICT_H_
