// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_COLOR_H_
#define NATIVEUI_GFX_COLOR_H_

#include <stdint.h>

namespace nu {

// A class to represent colors.
class Color {
 public:
  Color(unsigned a, unsigned r, unsigned g, unsigned b)
      : value_((a << 24) | (r << 16) | (g << 8) | (b << 0)) {}
  Color(unsigned r, unsigned g, unsigned b)
      : Color(0xFF, r, g, b) {}
  Color() : value_(0) {}

  unsigned a() const { return ((value_) >> 24) & 0xFF; }
  unsigned r() const { return ((value_) >> 16) & 0xFF; }
  unsigned g() const { return ((value_) >>  8) & 0xFF; }
  unsigned b() const { return ((value_) >>  0) & 0xFF; }

  bool operator==(Color other) const {
    return value_ == other.value_;
  }
  bool operator!=(Color other) const {
    return value_ != other.value_;
  }
  bool operator<(Color other) const {
    return value_ < other.value_;
  }
  bool operator>(Color other) const {
    return value_ > other.value_;
  }

 private:
  uint32_t value_;
};

// Available system colors.
enum class ThemeColor {
  Text,
};

// Returns the color of a system element.
Color GetThemeColor(ThemeColor theme);

}  // namespace nu

#endif  // NATIVEUI_GFX_COLOR_H_
