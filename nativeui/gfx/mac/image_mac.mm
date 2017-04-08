// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/image.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"

namespace nu {

Image::Image(const std::string& path)
    : scale_factor_(1.f),
      image_([[NSImage alloc]
                 initWithContentsOfFile:base::SysUTF8ToNSString(path)]) {
  // Compute the scale factor from actual NSImageRep.
  NSArray* reps = [image_ representations];
  if ([reps count] > 0) {
    float lw = [image_ size].width;
    float pw = [static_cast<NSImageRep*>([reps objectAtIndex:0]) pixelsWide];
    if (lw > 0 && pw > 0)
      scale_factor_ = pw / lw;
    // NSImage caculates the DPI from the image automatically, which may not be
    // the same with the DPI set by the @2x suffix, in this case we need to set
    // size of NSImage to match the scale factor.
    float expected = GetScaleFactorFromFilePath(path);
    if (scale_factor_ != expected) {
      float ph = [static_cast<NSImageRep*>([reps objectAtIndex:0]) pixelsHigh];
      [image_ setSize:NSMakeSize(pw / expected, ph / expected)];
      scale_factor_ = expected;
    }
  }
}

Image::~Image() {
  [image_ release];
}

SizeF Image::GetSize() const {
  return SizeF([image_ size]);
}

NativeImage Image::GetNative() const {
  return image_;
}

}  // namespace nu
