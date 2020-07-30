// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/image.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/scoped_cftyperef.h"
#include "base/strings/pattern.h"
#include "base/strings/sys_string_conversions.h"

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

SizeF Image::GetSize() const {
  return SizeF([image_ size]);
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
