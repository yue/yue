// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/table/nu_wrapped_value.h"

@implementation NUWrappedValue

+ (instancetype)valueWithValue:(base::Value)value {
  NUWrappedValue* instance = [[NUWrappedValue alloc] init];
  instance->value_ = std::move(value);
  return [instance autorelease];
}

- (base::Value)pass {
  return std::move(value_);
}

@end
