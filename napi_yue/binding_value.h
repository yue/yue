// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NAPI_YUE_BINDING_VALUE_H_
#define NAPI_YUE_BINDING_VALUE_H_

#include "base/values.h"
#include "third_party/kizunapi/kizunapi.h"

namespace ki {

template<>
struct Type<base::Value> {
  static constexpr const char* name = "Value";
  static napi_status ToNode(napi_env env,
                            const base::Value& value,
                            napi_value* result);
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              base::Value* out);
};

template<>
struct Type<const base::Value*> {
  static constexpr const char* name = "Value";
  static napi_status ToNode(napi_env env,
                            const base::Value* value,
                            napi_value* result) {
    return ConvertToNode(env, *value, result);
  }
};

}  // namespace ki

#endif  // NAPI_YUE_BINDING_VALUE_H_
