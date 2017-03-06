// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_DICT_H_
#define V8BINDING_DICT_H_

#include "v8binding/types.h"

namespace vb {

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
             ToV8Data(context, value));
  return true;
}

// Allow setting arbitrary key/value pairs.
template<typename Dict, typename Key, typename Value, typename... ArgTypes>
inline bool Set(v8::Local<v8::Context> context, Dict dict,
                const Key& key, const Value& value,
                const ArgTypes&... args) {
  return Set(context, dict, key, value) &&
         Set(context, dict, args...);
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
