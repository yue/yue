// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/mac/image_mac.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"

namespace nu {

ImageMac::ImageMac(const std::string& path)
    : image_([[NSImage alloc]
                 initWithContentsOfFile:base::SysUTF8ToNSString(path)]) {
}

ImageMac::~ImageMac() {
}

Size ImageMac::GetSize() const {
  return Size([image_ size]);
}

NativeImage ImageMac::GetNative() const {
  return image_.get();
}

}  // namespace nu
