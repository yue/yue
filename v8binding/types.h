// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_TYPES_H_
#define V8BINDING_TYPES_H_

#include <string>

#include "base/strings/string_piece.h"
#include "v8.h"  // NOLINT(build/include)

namespace vb {

template<typename T, typename Enable = void>
struct Type {};

template<>
struct Type<int> {
  static constexpr const char* name = "Integer";
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          int32_t value) {
    return v8::Integer::New(context->GetIsolate(), value);
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     int32_t* out) {
    if (!value->IsInt32())
      return false;
    *out = value->Int32Value();
    return true;
  }
};

template<>
struct Type<float> {
  static constexpr const char* name = "Number";
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          float value) {
    return v8::Number::New(context->GetIsolate(), value);
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     float* out) {
    if (!value->IsNumber())
      return false;
    *out = static_cast<float>(value->NumberValue());
    return true;
  }
};

template<>
struct Type<bool> {
  static constexpr const char* name = "Boolean";
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          bool value) {
    return v8::Boolean::New(context->GetIsolate(), value);
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     bool* out) {
    if (!value->IsBoolean())
      return false;
    *out = value->BooleanValue();
    return true;
  }
};

template<>
struct Type<std::string> {
  static constexpr const char* name = "String";
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
  static constexpr const char* name = "String";
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          const char* value) {
    return v8::String::NewFromUtf8(context->GetIsolate(), value,
                                   v8::NewStringType::kNormal,
                                   -1).ToLocalChecked();
  }
};

template<size_t n>
struct Type<const char[n]> {
  static constexpr const char* name = "String";
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

template<typename T>
struct Type<v8::Local<T>, typename std::enable_if<std::is_base_of<
                              v8::Value, T>::value>::type> {
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          v8::Local<T> value) {
    return value;
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     v8::Local<T>* out) {
    *out = v8::Local<T>::Cast(value);
    return out->IsEmpty();
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

// Create V8 symbol.
template<size_t n>
inline v8::Local<v8::String> ToV8Symbol(v8::Local<v8::Context> context,
                                        const char (&str)[n]) {
  return v8::String::NewFromUtf8(context->GetIsolate(), str,
                                 v8::NewStringType::kInternalized,
                                 n - 1).ToLocalChecked();
}

}  // namespace vb

#endif  // V8BINDING_TYPES_H_
