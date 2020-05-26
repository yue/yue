// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/canvas.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/scoped_cftyperef.h"
#include "nativeui/gfx/mac/painter_mac.h"

namespace nu {

// static
NativeBitmap Canvas::PlatformCreateBitmap(const SizeF& size,
                                          float scale_factor) {
  // We have to create a flipped NSGraphicsContext by using the deprecated
  // graphicsContextWithGraphicsPort API, which only accepts a CGContextRef.
  // So the only solution left to us is to use CGContextRef instead of
  // NSBitmapImageRep as bitmap representation.
  base::ScopedCFTypeRef<CGColorSpaceRef> color_space(
        CGColorSpaceCreateDeviceRGB());
  CGContextRef bitmap = CGBitmapContextCreate(
      nullptr, size.width() * scale_factor, size.height() * scale_factor,
      8, 0, color_space,
       kCGBitmapByteOrder32Host | kCGImageAlphaPremultipliedFirst);
  return bitmap;
}

// static
void Canvas::PlatformDestroyBitmap(NativeBitmap bitmap) {
  CGContextRelease(bitmap);
}

// static
Painter* Canvas::PlatformCreatePainter(NativeBitmap bitmap,
                                       const SizeF& size,
                                       float scale_factor) {
  return new PainterMac(bitmap, size, scale_factor);
}

}  // namespace nu
