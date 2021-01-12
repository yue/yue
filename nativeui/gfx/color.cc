// Copyright 2016 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/color.h"

#include <cmath>
#include <vector>

#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"

namespace nu {

namespace {

const uint32_t kColorWhite = 0xFFFFFFFF;

uint32_t ParseHexColor(const std::string& color_string) {
  // Check the string for incorrect formatting.
  if (color_string.empty() || color_string[0] != '#')
    return kColorWhite;

  // Prepend FF if alpha channel is not specified.
  std::string source = color_string.substr(1);
  if (source.size() == 3)
    source.insert(0, "F");
  else if (source.size() == 6)
    source.insert(0, "FF");

  // Convert the string from #FFF format to #FFFFFF format.
  std::string formatted_color;
  if (source.size() == 4) {
    for (size_t i = 0; i < 4; ++i) {
      formatted_color += source[i];
      formatted_color += source[i];
    }
  } else if (source.size() == 8) {
    formatted_color = source;
  } else {
    return kColorWhite;
  }

  // Convert the string to an integer and make sure it is in the correct value
  // range.
  std::vector<uint8_t> bytes;
  if (!base::HexStringToBytes(formatted_color, &bytes))
    return kColorWhite;

  return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) |
         (bytes[3] << 0);
}

// Assumes sRGB.
inline float Linearize(float eight_bit_component) {
  const float component = eight_bit_component / 255.0f;
  // The W3C link in the header uses 0.03928 here.  See
  // https://en.wikipedia.org/wiki/SRGB#Theory_of_the_transformation for
  // discussion of why we use this value rather than that one.
  return (component <= 0.04045f) ? (component / 12.92f)
                                 : pow((component + 0.055f) / 1.055f, 2.4f);
}

}  // namespace

Color::Color(const std::string& hex) : value_(ParseHexColor(hex)) {}

float Color::GetRelativeLuminance() const {
  return (0.2126f * Linearize(r())) +
         (0.7152f * Linearize(g())) +
         (0.0722f * Linearize(b()));
}

std::string Color::ToString() const {
  return base::StringPrintf("rgba(%d, %d, %d, %d)", r(), g(), b(), a());
}

}  // namespace nu
