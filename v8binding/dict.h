// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_DICT_H_
#define V8BINDING_DICT_H_

#include <utility>

#include "v8binding/ref_method.h"

namespace vb {

namespace internal {

// Define how can a type be converted to a field of ObjectTemplate.
template<typename T, typename Enable = void>
struct ToV8Data {
  static inline v8::Local<v8::Data> Do(v8::Local<v8::Context> context,
                                       T&& value) {
    return ToV8(context, std::forward<T>(value));
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
                                       const RefMethodRef<T>& callback) {
#ifndef NDEBUG
    internal::FunctionTemplateCreated();
#endif
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
                Key&& key, Value&& value) {
  auto result = object->Set(context,
                            ToV8(context, std::forward<Key>(key)),
                            ToV8(context, std::forward<Value>(value)));
  return !result.IsNothing() && result.FromJust();
}

// Helper for setting ObjectTemplate.
template<typename Key, typename Value>
inline bool Set(v8::Local<v8::Context> context,
                v8::Local<v8::ObjectTemplate> templ,
                Key&& key, Value&& value) {
  templ->Set(ToV8(context, std::forward<Key>(key)).template As<v8::String>(),
             internal::ToV8Data<Value>::Do(context,
                                           std::forward<Value>(value)));
  return true;
}

// Allow setting arbitrary key/value pairs.
template<typename Dict, typename Key, typename Value, typename... ArgTypes>
inline bool Set(v8::Local<v8::Context> context, Dict dict,
                Key&& key, Value&& value, ArgTypes&&... args) {
  bool result = Set(context, dict,
                    std::forward<Key>(key), std::forward<Value>(value));
  result &= Set(context, dict, std::forward<ArgTypes>(args)...);
  return result;
}

// Helper for getting from Object.
template<typename Key, typename Value>
inline bool Get(v8::Local<v8::Context> context, v8::Local<v8::Object> object,
                Key&& key, Value* out) {
  // Check key before get, otherwise this method will always return true for
  // Key == v8::Local<v8::Value>.
  v8::Local<v8::Value> v8_key = ToV8(context, std::forward<Key>(key));
  v8::Maybe<bool> has = object->Has(context, v8_key);
  if (has.IsNothing() || !has.FromJust())
    return false;
  v8::Local<v8::Value> value;
  if (!object->Get(context, v8_key).ToLocal(&value))
    return false;
  return FromV8(context, value, out);
}

// Allow getting arbitrary values.
template<typename Key, typename Value, typename... ArgTypes>
inline bool Get(v8::Local<v8::Context> context, v8::Local<v8::Object> object,
                Key&& key, Value* out, ArgTypes&&... args) {
  bool success = Get(context, object, std::forward<Key>(key), out);
  success &= Get(context, object, std::forward<ArgTypes>(args)...);
  return success;
}

// Like Get but ignore unexist keys.
template<typename Key, typename Value>
inline bool ReadOptions(v8::Local<v8::Context> context,
                        v8::Local<v8::Object> object,
                        Key&& key, Value* out) {
  v8::Local<v8::Value> v8_key = ToV8(context, std::forward<Key>(key));
  v8::Maybe<bool> has = object->Has(context, v8_key);
  if (has.IsNothing() || !has.FromJust())
    return true;
  v8::Local<v8::Value> value;
  if (!object->Get(context, v8_key).ToLocal(&value))
    return false;
  return FromV8(context, value, out);
}
template<typename Key, typename Value, typename... ArgTypes>
inline bool ReadOptions(v8::Local<v8::Context> context,
                        v8::Local<v8::Object> object,
                        Key&& key, Value* out, ArgTypes&&... args) {
  bool success = ReadOptions(context, object, std::forward<Key>(key), out);
  success &= ReadOptions(context, object, std::forward<ArgTypes>(args)...);
  return success;
}

// Return a hidden map attached to object.
v8::Local<v8::Map> GetAttachedTable(v8::Local<v8::Context> context,
                                    v8::Local<v8::Object> object,
                                    const base::StringPiece& key);
v8::Local<v8::Map> GetAttachedTable(v8::Local<v8::Context> context,
                                    v8::Local<v8::Object> object,
                                    void* key);

}  // namespace vb

#endif  // V8BINDING_DICT_H_
