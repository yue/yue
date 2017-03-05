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
  static constexpr const char* name = "string";
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          const std::string& value) {
    return v8::String::NewFromUtf8(
        context->GetIsolate(), value.data(),
        v8::NewStringType::kNormal,
        static_cast<uint32_t>(value.length())).ToLocalChecked();
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     std::string* out) {
    if (!value->IsString())
      return false;
    v8::Local<v8::String> str = v8::Local<v8::String>::Cast(value);
    int length = str->Utf8Length();
    out->resize(length);
    str->WriteUtf8(&(*out)[0], length, NULL, v8::String::NO_NULL_TERMINATION);
    return true;
  }
};

template<>
struct Type<const char*> {
  static constexpr const char* name = "string";
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          const char* value) {
    return v8::String::NewFromUtf8(context->GetIsolate(), value,
                                   v8::NewStringType::kNormal,
                                   -1).ToLocalChecked();
  }
};

template<size_t n>
struct Type<const char[n]> {
  static constexpr const char* name = "string";
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          const char* value) {
    return v8::String::NewFromUtf8(context->GetIsolate(), value,
                                   v8::NewStringType::kNormal,
                                   n - 1).ToLocalChecked();
  }
};

template<>
struct Type<v8::Local<v8::Value>> {
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          v8::Local<v8::Value> value) {
    return value;
  }
};

template<>
struct Type<v8::Local<v8::Function>> {
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          v8::Local<v8::Function> value) {
    return value;
  }
};

template<>
struct Type<v8::Local<v8::External>> {
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          v8::Local<v8::External> value) {
    return value;
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     v8::Local<v8::External>* out) {
    if (!value->IsExternal())
      return false;
    *out = v8::Local<v8::External>::Cast(value);
    return true;
  }
};

// Helpers
template<typename T>
inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                 const T& type) {
  return Type<T>::ToV8(context, type);
}

template<typename T>
inline v8::Local<v8::Data> ToV8Data(v8::Local<v8::Context> context,
                                    const T& type) {
  return Type<T>::ToV8Data(context, type);
}

template<typename T>
inline bool FromV8(v8::Local<v8::Context> context,
                   v8::Local<v8::Value> value,
                   T* out) {
  return Type<T>::FromV8(context, value, out);
}

// Optimized version for string iterals.
template<size_t n>
inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                 const char (&str)[n]) {
  return Type<const char[n]>::ToV8(context, str);
}

}  // namespace vb

#endif  // V8BINDING_TYPES_H_
