// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_ARGUMENTS_H_
#define V8BINDING_ARGUMENTS_H_

#include <string>

#include "v8binding/types.h"

namespace vb {

// Arguments is a wrapper around v8::FunctionCallbackInfo that integrates
// with Converter to make it easier to marshall arguments and return values
// between V8 and C++.
class Arguments {
 public:
  Arguments();
  explicit Arguments(const v8::FunctionCallbackInfo<v8::Value>& info);
  ~Arguments();

  template<typename T>
  bool GetHolder(T* out) {
    return FromV8(isolate_->GetCurrentContext(), info_->Holder(), out);
  }

  template<typename T>
  bool GetData(T* out) {
    return FromV8(isolate_->GetCurrentContext(), info_->Data(), out);
  }

  template<typename T>
  bool GetNext(T* out) {
    if (next_ >= info_->Length()) {
      insufficient_arguments_ = true;
      return false;
    }
    v8::Local<v8::Value> val = (*info_)[next_++];
    return FromV8(isolate_->GetCurrentContext(), val, out);
  }

  v8::Local<v8::Object> This() const {
    return info_->This();
  }

  v8::Local<v8::Context> GetContext() const {
    return isolate_->GetCurrentContext();
  }

  int Length() const {
    return info_->Length();
  }

  template<typename T>
  void Return(const T& val) {
    info_->GetReturnValue().Set(ToV8(isolate_->GetCurrentContext(), val));
  }

  void ThrowError(const char* target_type_name) const;

  const v8::FunctionCallbackInfo<v8::Value>& info() const { return *info_; }

  v8::Isolate* isolate() const { return isolate_; }

 private:
  v8::Isolate* isolate_;
  const v8::FunctionCallbackInfo<v8::Value>* info_;
  int next_;
  bool insufficient_arguments_;
};

}  // namespace vb

#endif  // V8BINDING_ARGUMENTS_H_
