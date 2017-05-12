// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/font.h"

#include <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"

namespace nu {

namespace {

// Convert between Cocoa font weight and our own font weight enums.
CGFloat FontWeightToNS(Font::Weight weight) {
  switch (weight) {
    case Font::Weight::ExtraLight:
      return -0.8;  // NSFontWeightNSFontWeightUltraLight
    case Font::Weight::Thin:
      return -0.6;  // NSFontWeightThin
    case Font::Weight::Light:
      return -0.4;  // NSFontWeightLight
    case Font::Weight::Medium:
      return 0.23;  // NSFontWeightMedium
    case Font::Weight::SemiBold:
      return 0.3;  // NSFontWeightSemibold
    case Font::Weight::Bold:
      return 0.4;  // NSFontWeightBold
    case Font::Weight::ExtraBold:
      return 0.56;  // NSFontWeightHeavy
    case Font::Weight::Black:
      return 0.62;  // NSFontWeightBlack
    default:
      return 0;  // NSFontWeightRegular
  }
}

Font::Weight FontWeightFromNS(CGFloat weight) {
  if (weight <= -0.8)  // NSFontWeightNSFontWeightUltraLight
    return Font::Weight::ExtraLight;
  else if (weight <= -0.6)  // NSFontWeightThin
    return Font::Weight::Thin;
  else if (weight <= -0.4)  // NSFontWeightLight
    return Font::Weight::Light;
  else if (weight <= 0)  // NSFontWeightRegular
    return Font::Weight::Normal;
  else if (weight <= 0.23)  // NSFontWeightMedium
    return Font::Weight::Medium;
  else if (weight <= 0.3)  // NSFontWeightSemibold
    return Font::Weight::SemiBold;
  else if (weight <= 0.4)  // NSFontWeightBold
    return Font::Weight::Bold;
  else if (weight <= 0.56)  // NSFontWeightHeavy
    return Font::Weight::ExtraBold;
  else if (weight <= 0.62)  // NSFontWeightBlack
    return Font::Weight::Black;
  else  // we don't have a definition for this weight, just use Black.
    return Font::Weight::Black;
}

// Returns an autoreleased NSFont created with the passed-in specifications.
NSFont* NSFontWithSpec(const std::string& font_name, float font_size,
                       Font::Weight font_weight, Font::Style font_style) {
  NSFontSymbolicTraits trait_bits = 0;
  if (font_style == Font::Style::Italic)
    trait_bits |= NSFontItalicTrait;
  NSDictionary* traits = @{
    NSFontSymbolicTrait : @(trait_bits),
    NSFontWeightTrait : @(FontWeightToNS(font_weight)),
  };

  NSString* family = base::SysUTF8ToNSString(font_name);
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

Font::Font(const std::string& name, float size, Weight weight, Style style)
    : font_([NSFontWithSpec(name, size, weight, style) retain]) {}

Font::~Font() {
  [font_ release];
}

std::string Font::GetName() const {
  return base::SysNSStringToUTF8([font_ familyName]);
}

float Font::GetSize() const {
  return [font_ pointSize];
}

Font::Weight Font::GetWeight() const {
  id traits = [[font_ fontDescriptor] objectForKey:NSFontTraitsAttribute];
  return FontWeightFromNS([traits[NSFontWeightTrait] doubleValue]);
}

Font::Style Font::GetStyle() const {
  NSFontSymbolicTraits traits = [[font_ fontDescriptor] symbolicTraits];
  if (traits & NSFontItalicTrait)
    return Font::Style::Italic;
  else
    return Font::Style::Normal;
}

NativeFont Font::GetNative() const {
  return font_;
}

}  // namespace nu
