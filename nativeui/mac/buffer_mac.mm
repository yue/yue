// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/buffer.h"

#import <Cocoa/Cocoa.h>

namespace nu {

NSData* Buffer::ToNSData() const {
  return [NSData dataWithBytesNoCopy:content_
                              length:size_
                        freeWhenDone:NO];
}

}  // namespace nu
