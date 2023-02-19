// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "napi_yue/binding_value.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "base/notreached.h"

namespace ki {

// static
napi_status Type<base::Value>::ToNode(napi_env env,
                                      const base::Value& value,
                                      napi_value* result) {
  switch (value.type()) {
    case base::Value::Type::NONE:
      return ConvertToNode(env, nullptr, result);
    case base::Value::Type::BOOLEAN:
      return ConvertToNode(env, value.GetBool(), result);
    case base::Value::Type::INTEGER:
      return ConvertToNode(env, value.GetInt(), result);
    case base::Value::Type::DOUBLE:
      return ConvertToNode(env, value.GetDouble(), result);
    case base::Value::Type::STRING:
      return ConvertToNode(env, value.GetString(), result);
    case base::Value::Type::BINARY:
      return napi_create_buffer_copy(
          env, value.GetBlob().size(), value.GetBlob().data(), nullptr, result);
    case base::Value::Type::DICTIONARY: {
      napi_status s = napi_create_object(env, result);
      if (s == napi_ok) {
        for (const auto it : value.GetDict()) {
          if (!Set(env, *result,
                   ki::ToNode(env, it.first), ki::ToNode(env, it.second)))
            break;
        }
      }
      return s;
    }
    case base::Value::Type::LIST: {
      size_t length = value.GetList().size();
      napi_status s = napi_create_array_with_length(env, length, result);
      if (s == napi_ok) {
        for (size_t i = 0; i < length; ++i) {
          s = napi_set_element(env, *result,
                               i, ki::ToNode(env, value.GetList()[i]));
          if (s != napi_ok) break;
        }
      }
      return s;
    }
  }
  NOTREACHED();
  return napi_generic_failure;
}

// static
napi_status Type<base::Value>::FromNode(napi_env env,
                                        napi_value value,
                                        base::Value* out) {
  napi_valuetype type;
  napi_status s = napi_typeof(env, value, &type);
  if (s != napi_ok)
    return s;
  switch (type) {
    case napi_undefined:
    case napi_null:
      *out = base::Value();
      break;
    case napi_boolean:
      *out = base::Value(FromNodeTo<bool>(env, value));
      break;
    case napi_number:
      *out = base::Value(FromNodeTo<double>(env, value));
      break;
    case napi_string:
    case napi_symbol:
      *out = base::Value(FromNodeTo<std::string>(env, value));
      break;
    case napi_object:
      if (IsArray(env, value)) {
        std::vector<base::Value> list;
        s = ConvertFromNode(env, value, &list);
        if (s != napi_ok) break;
        base::Value::List storage;
        storage.reserve(list.size());
        for (auto& it : list)
          storage.Append(std::move(it));
        *out = base::Value(std::move(storage));
        break;
      } else {
        std::map<std::string, base::Value> map;
        s = ConvertFromNode(env, value, &map);
        if (s != napi_ok) break;
        base::Value::Dict storage;
        for (auto& it : map)
          storage.Set(it.first, std::move(it.second));
        *out = base::Value(std::move(storage));
        break;
      }
      break;
    default:
      NOTREACHED() << "Unsupported JavaScript type: "
                   << NodeTypeToString(env, value);
      s = napi_generic_failure;
  }
  return s;
}

}  // namespace ki
