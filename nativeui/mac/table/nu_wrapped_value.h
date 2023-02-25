// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_TABLE_NU_WRAPPED_VALUE_H_
#define NATIVEUI_MAC_TABLE_NU_WRAPPED_VALUE_H_

#import <Cocoa/Cocoa.h>

#include "base/values.h"

// Helper to transfer base::Value in/out Cocoa.
@interface NUWrappedValue : NSObject {
 @private
  base::Value value_;
}
+ (instancetype)valueWithValue:(base::Value)value;
- (base::Value)pass;
@end

#endif  // NATIVEUI_MAC_TABLE_NU_WRAPPED_VALUE_H_
