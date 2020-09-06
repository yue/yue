// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/value_conversion.h"

#import <Cocoa/Cocoa.h>

#include "base/memory/ptr_util.h"
#include "base/strings/sys_string_conversions.h"
#include "base/values.h"

namespace nu {

// Convert common NSValue to base::Value.
base::Value NSValueToBaseValue(id value) {
  if (!value || [value isKindOfClass:[NSNull class]]) {
    return base::Value();
  } else if ([value isKindOfClass:[NSString class]]) {
    return base::Value(base::SysNSStringToUTF8(value));
  } else if ([value isKindOfClass:[NSNumber class]]) {
    const char* objc_type = [value objCType];
    if (strcmp(objc_type, @encode(BOOL)) == 0 ||
        strcmp(objc_type, @encode(char)) == 0)
      return base::Value(static_cast<bool>([value boolValue]));
    else if (strcmp(objc_type, @encode(double)) == 0 ||
             strcmp(objc_type, @encode(float)) == 0)
      return base::Value([value doubleValue]);
    else
      return base::Value([value intValue]);
  } else if ([value isKindOfClass:[NSArray class]]) {
    base::Value::ListStorage arr;
    arr.reserve([value count]);
    for (id item in value)
      arr.push_back(NSValueToBaseValue(item));
    return base::Value(std::move(arr));
  } else if ([value isKindOfClass:[NSDictionary class]]) {
    base::DictionaryValue dict;
    for (id key in value) {
      std::string str_key = base::SysNSStringToUTF8(
          [key isKindOfClass:[NSString class]] ? key : [key description]);
      auto vval = std::make_unique<base::Value>(
          NSValueToBaseValue([value objectForKey:key]));
      dict.SetWithoutPathExpansion(str_key.c_str(), std::move(vval));
    }
    return std::move(dict);
  } else {
    return base::Value(base::SysNSStringToUTF8([value description]));
  }
}

}  // namespace nu
