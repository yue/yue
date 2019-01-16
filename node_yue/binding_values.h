// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NODE_YUE_BINDING_VALUES_H_
#define NODE_YUE_BINDING_VALUES_H_

#include "base/values.h"
#include "v8binding/v8binding.h"

namespace vb {

template<>
struct Type<base::Value> {
  static constexpr const char* name = "Value";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const base::Value& value);
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     base::Value* out);
};

template<>
struct Type<const base::Value*> {
  static constexpr const char* name = "Value";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const base::Value* value);
};

}  // namespace vb

#endif  // NODE_YUE_BINDING_VALUES_H_
