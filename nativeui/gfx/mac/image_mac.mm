// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/image.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/scoped_cftyperef.h"
#include "base/mac/scoped_nsobject.h"
#include "base/strings/pattern.h"
#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/geometry/rect_f.h"

namespace nu {

namespace {

// NSImage uses clamped duration by default, which makes NSImageView useless,
// the only way to get true duration time is to use CGImage.
std::vector<float> GetFrameDurations(NSBitmapImageRep* bitmap,
                                     CGImageSourceRef image) {
  std::vector<float> durations;
  if (!image)
    return durations;
  NSNumber* frames = [bitmap valueForProperty:NSImageFrameCount];
  int frames_count = [frames intValue];
  durations.resize(frames_count);
  for (int i = 0; i < frames_count; ++i) {
    CFDictionaryRef cfdict = CGImageSourceCopyPropertiesAtIndex(image, i, nullptr);
    NSDictionary* dict = CFBridgingRelease(cfdict);
    NSNumber* delay =
        [[dict objectForKey:(__bridge NSString *)kCGImagePropertyGIFDictionary]
              objectForKey:(__bridge NSString *)kCGImagePropertyGIFUnclampedDelayTime];
    durations[i] = delay ? [delay floatValue] * 1000 : 100;
  }
  return durations;
}

Buffer EncodeImage(NSImage* image,
                   NSBitmapImageFileType type,
                   NSDictionary* properties) {
  CGImageRef cg_image =
      [image CGImageForProposedRect:nullptr context:nil hints:nil];
  base::scoped_nsobject<NSBitmapImageRep> rep(
      [[NSBitmapImageRep alloc] initWithCGImage:cg_image]);
  NSData* data = [[rep representationUsingType:type
                                    properties:properties] retain];
  return Buffer::TakeOver(const_cast<void*>(data.bytes), data.length,
                          [data](void*) { [data release]; });
}

}  // namespace

Image::Image() : image_([[NSImage alloc] init]) {}

Image::Image(const base::FilePath& p)
    : image_([[NSImage alloc]
                 initWithContentsOfFile:base::SysUTF8ToNSString(p.value())]) {
  if (!image_)
    image_ = [[NSImage alloc] init];
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
    float expected = GetScaleFactorFromFilePath(p);
    if (scale_factor_ != expected) {
      float ph = [static_cast<NSImageRep*>([reps objectAtIndex:0]) pixelsHigh];
      [image_ setSize:NSMakeSize(pw / expected, ph / expected)];
      scale_factor_ = expected;
    }
  }
  // Read image with CGImage for animations.
  NSBitmapImageRep* rep = GetAnimationRep();
  if (rep) {
    NSString* u = base::SysUTF8ToNSString(p.value());
    base::ScopedCFTypeRef<CGImageSourceRef> source(
        CGImageSourceCreateWithURL((__bridge CFURLRef)[NSURL fileURLWithPath:u],
                                   nullptr));
    durations_ = GetFrameDurations(rep, source);
  }
  // Is template image.
  if (base::MatchPattern(p.value(), "*Template.*") ||
      base::MatchPattern(p.value(), "*Template@*x.*")) {
    [image_ setTemplate:YES];
  }
}

Image::Image(const Buffer& buffer, float scale_factor)
    : scale_factor_(scale_factor),
      image_([[NSImage alloc] initWithData:buffer.ToNSData()]) {
  if (!image_)
    image_ = [[NSImage alloc] init];
  if (scale_factor_ != 1.f) {
    // Set the scale factor.
    NSArray* reps = [image_ representations];
    if ([reps count] > 0) {
      NSImageRep* rep = static_cast<NSImageRep*>([reps objectAtIndex:0]);
      [image_ setSize:NSMakeSize([rep pixelsWide] / scale_factor_,
                                 [rep pixelsHigh] / scale_factor_)];
    }
  }
  // Read image with CGImage for animations.
  NSBitmapImageRep* rep = GetAnimationRep();
  if (rep) {
    base::ScopedCFTypeRef<CGImageSourceRef> source(
        CGImageSourceCreateWithData((__bridge CFDataRef)buffer.ToNSData(),
                                    nullptr));
    durations_ = GetFrameDurations(rep, source);
  }
}

Image::~Image() {
  [image_ release];
}

bool Image::IsEmpty() const {
  return [[image_ representations] count] == 0;
}

void Image::SetTemplate(bool is) {
  [image_ setTemplate:is];
}

bool Image::IsTemplate() const {
  return [image_ isTemplate];
}

SizeF Image::GetSize() const {
  return SizeF([image_ size]);
}

Image* Image::Tint(Color color) const {
  NSImage* tinted = [[NSImage alloc] initWithSize:[image_ size]];
  [tinted lockFocus];
  [image_ drawAtPoint:NSZeroPoint
             fromRect:NSZeroRect
            operation:NSCompositingOperationSourceOver
             fraction:1.0];
  [color.ToNSColor() set];
  NSRectFillUsingOperation(RectF(GetSize()).ToCGRect(),
                           NSCompositingOperationSourceAtop);
  [tinted unlockFocus];
  return new Image(tinted, scale_factor_);
}

Image* Image::Resize(SizeF new_size, float scale_factor) const {
  base::scoped_nsobject<NSBitmapImageRep> rep([[NSBitmapImageRep alloc]
      initWithBitmapDataPlanes:nullptr
                    pixelsWide:new_size.width() * scale_factor
                    pixelsHigh:new_size.height() * scale_factor
                 bitsPerSample:8
               samplesPerPixel:4
                      hasAlpha:YES
                      isPlanar:NO
                colorSpaceName:NSCalibratedRGBColorSpace
                   bytesPerRow:0
                  bitsPerPixel:0]);
  [rep setSize:new_size.ToCGSize()];

  NSGraphicsContext* context =
      [NSGraphicsContext graphicsContextWithBitmapImageRep:rep];
  [NSGraphicsContext saveGraphicsState];
  [NSGraphicsContext setCurrentContext:context];
  [image_ drawInRect:RectF(new_size).ToCGRect()
            fromRect:NSZeroRect
           operation:NSCompositingOperationCopy fraction:1.0];
  [NSGraphicsContext restoreGraphicsState];

  NSImage* resized = [[NSImage alloc] initWithSize:new_size.ToCGSize()];
  [resized addRepresentation:rep];
  return new Image(resized, scale_factor);
}

Buffer Image::ToPNG() const {
  return EncodeImage(image_, NSBitmapImageFileTypePNG, nullptr);
}

Buffer Image::ToJPEG(int quality) const {
  NSDictionary* options = @{NSImageCompressionFactor: @(quality / 100.f)};
  return EncodeImage(image_, NSBitmapImageFileTypePNG, options);
}

NSBitmapImageRep* Image::GetAnimationRep() const {
  for (NSBitmapImageRep* rep in [image_ representations]) {
    if (![rep isKindOfClass:[NSBitmapImageRep class]])
      continue;
    NSNumber* frames = [rep valueForProperty:NSImageFrameCount];
    if (frames && [frames intValue] > 1)
      return rep;
  }
  return nullptr;
}

float Image::GetAnimationDuration(int index) const {
  return durations_[index];
}

}  // namespace nu
