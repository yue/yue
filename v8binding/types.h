// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_TYPES_H_
#define V8BINDING_TYPES_H_

#include "base/strings/string_piece.h"
#include "v8.h"  // NOLINT(build/include)

namespace vb {

template<typename T, typename Enable = void>
struct Type {};

template<>
struct Type<std::string> {
  static inline bool ToV8(v8::Local<v8::Context> context,
                          const std::string value,
                          v8::Local<v8::Value>* out) {
    *out = v8::String::NewFromUtf8(
        context->GetIsolate(), value.data(),
        v8::NewStringType::kNormal,
        static_cast<uint32_t>(value.length())).ToLocalChecked();
    return true;
  }
};

template<>
struct Type<v8::Local<v8::Value>> {
  static inline bool ToV8(v8::Local<v8::Context> context,
                          v8::Local<v8::Value> value,
                          v8::Local<v8::Value>* out) {
    *out = value;
    return true;
  }
};

template<>
struct Type<v8::Local<v8::Function>> {
  static inline bool ToV8(v8::Local<v8::Context> context,
                          v8::Local<v8::Function> value,
                          v8::Local<v8::Value>* out) {
    *out = value;
    return true;
  }
};

// Helpers
template<typename T>
inline bool ToV8(v8::Local<v8::Context> context,
                 const T& type,
                 v8::Local<v8::Value>* out) {
  return Type<T>::ToV8(context, type, out);
}

}  // namespace vb

#endif  // V8BINDING_TYPES_H_
