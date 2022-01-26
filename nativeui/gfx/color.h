// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_COLOR_H_
#define NATIVEUI_GFX_COLOR_H_

#include <stdint.h>

#include <string>

#include "build/build_config.h"
#include "nativeui/nativeui_export.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

#if defined(OS_LINUX)
typedef struct _GdkRGBA GdkRGBA;
#endif

#if defined(OS_MAC)
#ifdef __OBJC__
@class NSColor;
#else
class NSColor;
#endif
#endif

namespace nu {

// A class to represent colors.
class NATIVEUI_EXPORT Color {
 public:
  // Available names of colors.
  enum class Name {
    Text,
    DisabledText,
    Control,
    WindowBackground,
    Border,
  };

  static Color Get(Name name);

  explicit Color(const std::string& hex);
  explicit Color(uint32_t value) : value_(value) {}
  Color(unsigned a, unsigned r, unsigned g, unsigned b)
      : value_((a << 24) | (r << 16) | (g << 8) | (b << 0)) {}
  Color(unsigned r, unsigned g, unsigned b)
      : Color(0xFF, r, g, b) {}
  Color() : value_(0) {}

#if defined(OS_MAC)
  NSColor* ToNSColor() const;
#elif defined(OS_WIN)
  COLORREF ToCOLORREF() const;
#elif defined(OS_LINUX)
  GdkRGBA ToGdkRGBA() const;
#endif

  uint32_t value() const { return value_; }

  unsigned a() const { return ((value_) >> 24) & 0xFF; }
  unsigned r() const { return ((value_) >> 16) & 0xFF; }
  unsigned g() const { return ((value_) >>  8) & 0xFF; }
  unsigned b() const { return ((value_) >>  0) & 0xFF; }

  bool transparent() const { return a() == 0; }

  float GetRelativeLuminance() const;
  std::string ToString() const;

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

}  // namespace nu

#endif  // NATIVEUI_GFX_COLOR_H_
