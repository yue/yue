// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/mac/platform_font_mac.h"

#include <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"

namespace nu {

namespace {

// Returns an autoreleased NSFont created with the passed-in specifications.
NSFont* NSFontWithSpec(const std::string& font_name, int font_size) {
  NSFontSymbolicTraits trait_bits = 0;
  // The Mac doesn't support underline as a font trait, so just drop it.
  // (Underlines must be added as an attribute on an NSAttributedString.)
  NSDictionary* traits = @{ NSFontSymbolicTrait : @(trait_bits) };

  NSDictionary* attrs = @{
    NSFontFamilyAttribute : base::SysUTF8ToNSString(font_name),
    NSFontTraitsAttribute : traits
  };
  NSFontDescriptor* descriptor =
      [NSFontDescriptor fontDescriptorWithFontAttributes:attrs];
  NSFont* font = [NSFont fontWithDescriptor:descriptor size:font_size];
  if (font)
    return font;

  // Make one fallback attempt by looking up via font name rather than font
  // family name.
  attrs = @{
    NSFontNameAttribute : base::SysUTF8ToNSString(font_name),
    NSFontTraitsAttribute : traits
  };
  descriptor = [NSFontDescriptor fontDescriptorWithFontAttributes:attrs];
  return [NSFont fontWithDescriptor:descriptor size:font_size];
}

}  // namespace

PlatformFontMac::PlatformFontMac()
    : font_([[NSFont systemFontOfSize:[NSFont systemFontSize]] retain]) {
}

PlatformFontMac::PlatformFontMac(const std::string& font_name, int font_size)
    : font_([NSFontWithSpec(font_name, font_size) retain]) {}

PlatformFontMac::~PlatformFontMac() {
}

std::string PlatformFontMac::GetFontName() const {
  return base::SysNSStringToUTF8([font_ familyName]);
}

int PlatformFontMac::GetFontSize() const {
  return [font_ pointSize];
}

NativeFont PlatformFontMac::GetNativeFont() const {
  return font_.get();
}

// static
PlatformFont* PlatformFont::CreateDefault() {
  return new PlatformFontMac;
}

// static
PlatformFont* PlatformFont::CreateFromNameAndSize(const std::string& font_name,
                                                  int font_size) {
  return new PlatformFontMac(font_name, font_size);
}

}  // namespace nu
