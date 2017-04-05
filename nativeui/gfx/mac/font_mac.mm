// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/font.h"

#include <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"

namespace nu {

namespace {

// Returns an autoreleased NSFont created with the passed-in specifications.
NSFont* NSFontWithSpec(base::StringPiece font_name, float font_size) {
  NSFontSymbolicTraits trait_bits = 0;
  // The Mac doesn't support underline as a font trait, so just drop it.
  // (Underlines must be added as an attribute on an NSAttributedString.)
  NSDictionary* traits = @{ NSFontSymbolicTrait : @(trait_bits) };

  NSString* family = base::SysUTF8ToNSString(font_name.as_string());
  NSDictionary* attrs = @{
    NSFontFamilyAttribute : family,
    NSFontTraitsAttribute : traits,
  };
  NSFontDescriptor* descriptor =
      [NSFontDescriptor fontDescriptorWithFontAttributes:attrs];
  NSFont* font = [NSFont fontWithDescriptor:descriptor size:font_size];
  if (font)
    return font;

  // Make one fallback attempt by looking up via font name rather than font
  // family name.
  attrs = @{
    NSFontNameAttribute : family,
    NSFontTraitsAttribute : traits,
  };
  descriptor = [NSFontDescriptor fontDescriptorWithFontAttributes:attrs];
  font = [NSFont fontWithDescriptor:descriptor size:font_size];
  if (font)
    return font;

  // Otherwise return the default font.
  return [NSFont systemFontOfSize:font_size];
}

}  // namespace

Font::Font()
    : font_([[NSFont systemFontOfSize:[NSFont systemFontSize]] retain]) {
}

Font::Font(base::StringPiece name, float size)
    : font_([NSFontWithSpec(name, size) retain]) {}

Font::~Font() {
  [font_ release];
}

std::string Font::GetName() const {
  return base::SysNSStringToUTF8([font_ familyName]);
}

float Font::GetSize() const {
  return [font_ pointSize];
}

NativeFont Font::GetNative() const {
  return font_;
}

}  // namespace nu
