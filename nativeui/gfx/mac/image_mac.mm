// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/image.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"

namespace nu {

Image::Image(const std::string& path)
    : image_([[NSImage alloc]
                 initWithContentsOfFile:base::SysUTF8ToNSString(path)]) {
}

Image::~Image() {
  [image_ release];
}

Size Image::GetSize() const {
  return Size([image_ size]);
}

NativeImage Image::GetNative() const {
  return image_;
}

}  // namespace nu
