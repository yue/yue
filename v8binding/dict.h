// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_DICT_H_
#define V8BINDING_DICT_H_

#include "v8binding/types.h"

namespace vb {

class Dict {
 public:
  Dict(v8::Local<v8::Context> context, v8::Local<v8::Object> object);
  ~Dict();

  template<typename T>
  bool Set(const std::string& key, const T& value) {
    v8::Local<v8::Value> v8_key, v8_value;
    if (!ToV8(context_, value, &v8_value) || !ToV8(context_, key, &v8_key))
      return false;
    v8::Maybe<bool> result = object_->Set(context_, v8_key, v8_value);
    return !result.IsNothing() && result.FromJust();
  }

 private:
  v8::Local<v8::Context> context_;
  v8::Local<v8::Object> object_;

  DISALLOW_COPY_AND_ASSIGN(Dict);
};

}  // namespace vb

#endif  // V8BINDING_DICT_H_
