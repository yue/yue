// Copyright 2018 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NODE_YUE_V8_VALUE_CONVERTER_H_
#define NODE_YUE_V8_VALUE_CONVERTER_H_

#include <memory>

#include "v8binding/v8binding.h"

namespace base {
class Value;
}

namespace node_yue {

// Converts between v8::Value (JavaScript values in the v8 heap) and Chrome's
// values (from base/values.h). Lists and dictionaries are converted
// recursively.
//
// The JSON types (null, boolean, string, number, array, and object) as well as
// binary values are supported. For binary values, we convert to WebKit
// ArrayBuffers, and support converting from an ArrayBuffer or any of the
// ArrayBufferView subclasses (Uint8Array, etc.).
class V8ValueConverter {
 public:
  static std::unique_ptr<V8ValueConverter> Create();

  virtual ~V8ValueConverter() {}

  // If true, Date objects are converted into DoubleValues with the number of
  // seconds since Unix epoch.
  //
  // Otherwise they are converted into DictionaryValues with whatever additional
  // properties has been set on them.
  virtual void SetDateAllowed(bool val) = 0;

  // If true, RegExp objects are converted into StringValues with the regular
  // expression between / and /, for example "/ab?c/".
  //
  // Otherwise they are converted into DictionaryValues with whatever additional
  // properties has been set on them.
  virtual void SetRegExpAllowed(bool val) = 0;

  // If true, Function objects are converted into DictionaryValues with whatever
  // additional properties has been set on them.
  //
  // Otherwise they are treated as unsupported, see FromV8Value.
  virtual void SetFunctionAllowed(bool val) = 0;

  // If true, null values are stripped from objects. This is often useful when
  // converting arguments to extension APIs.
  virtual void SetStripNullFromObjects(bool val) = 0;

  // If true, treats -0 as an integer. Otherwise, -0 is converted to a double.
  virtual void SetConvertNegativeZeroToInt(bool val) = 0;

  // Converts a base::Value to a v8::Value.
  //
  // Unsupported types are replaced with null.  If an array or object throws
  // while setting a value, that property or item is skipped, leaving a hole in
  // the case of arrays.
  // TODO(dcheng): This should just take a const reference.
  virtual v8::Local<v8::Value> ToV8Value(
      const base::Value* value,
      v8::Local<v8::Context> context) const = 0;

  // Converts a v8::Value to base::Value.
  //
  // Unsupported types (unless explicitly configured) are not converted, so
  // this method may return NULL -- the exception is when converting arrays,
  // where unsupported types are converted to Value(Type::NONE).
  //
  // Likewise, if an object throws while converting a property it will not be
  // converted, whereas if an array throws while converting an item it will be
  // converted to Value(Type::NONE).
  virtual std::unique_ptr<base::Value> FromV8Value(
      v8::Local<v8::Value> value,
      v8::Local<v8::Context> context) const = 0;
};

}  // namespace node_yue

#endif  // NODE_YUE_V8_VALUE_CONVERTER_H_
