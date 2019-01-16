// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "node_yue/binding_values.h"

#include <memory>
#include <utility>

#include "node_yue/v8_value_converter.h"

namespace vb {

using node_yue::V8ValueConverter;

// static
v8::Local<v8::Value> Type<base::Value>::ToV8(v8::Local<v8::Context> context,
                                             const base::Value& value) {
  std::unique_ptr<V8ValueConverter> converter = V8ValueConverter::Create();
  return converter->ToV8Value(&value, context);
}

// statc
bool Type<base::Value>::FromV8(v8::Local<v8::Context> context,
                               v8::Local<v8::Value> value,
                               base::Value* out) {
  std::unique_ptr<V8ValueConverter> converter = V8ValueConverter::Create();
  std::unique_ptr<base::Value> result = converter->FromV8Value(value, context);
  if (!result)
    return false;
  *out = std::move(*result.release());
  return true;
}

// static
v8::Local<v8::Value> Type<const base::Value*>::ToV8(
    v8::Local<v8::Context> context,
    const base::Value* value) {
  std::unique_ptr<V8ValueConverter> converter = V8ValueConverter::Create();
  return converter->ToV8Value(value, context);
}

}  // namespace vb
