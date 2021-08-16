// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVEUI_GFX_GEOMETRY_SIZE_F_H_
#define NATIVEUI_GFX_GEOMETRY_SIZE_F_H_

#include <cmath>
#include <iosfwd>
#include <string>

#include "base/compiler_specific.h"
#include "nativeui/gfx/geometry/size.h"
#include "nativeui/nativeui_export.h"

namespace nu {

// A floating version of gfx::Size.
class NATIVEUI_EXPORT SizeF {
 public:
  SizeF() : width_(0.f), height_(0.f) {}
  SizeF(float width, float height)
      : width_(fmaxf(0, width)), height_(fmaxf(0, height)) {}
#if defined(OS_MAC)
  explicit SizeF(const CGSize& s);
#endif
  ~SizeF() {}

  explicit SizeF(const Size& size)
      : SizeF(static_cast<float>(size.width()),
              static_cast<float>(size.height())) {}

#if defined(OS_MAC)
  SizeF& operator=(const CGSize& s);
#endif

#if defined(OS_MAC)
  CGSize ToCGSize() const;
#endif

  float width() const { return width_; }
  float height() const { return height_; }

  void set_width(float width) { width_ = fmaxf(0, width); }
  void set_height(float height) { height_ = fmaxf(0, height); }

  float GetArea() const;

  void SetSize(float width, float height) {
    set_width(width);
    set_height(height);
  }

  void Enlarge(float grow_width, float grow_height);

  void SetToMin(const SizeF& other);
  void SetToMax(const SizeF& other);

  bool IsEmpty() const { return !width() || !height(); }

  void Scale(float scale) {
    Scale(scale, scale);
  }

  void Scale(float x_scale, float y_scale) {
    SetSize(width() * x_scale, height() * y_scale);
  }

  std::string ToString() const;

 private:
  float width_;
  float height_;
};

inline bool operator==(const SizeF& lhs, const SizeF& rhs) {
  return lhs.width() == rhs.width() && lhs.height() == rhs.height();
}

inline bool operator!=(const SizeF& lhs, const SizeF& rhs) {
  return !(lhs == rhs);
}

inline SizeF operator+(const SizeF& lhs, const SizeF& rhs) {
  SizeF result = lhs;
  result.Enlarge(rhs.width(), rhs.height());
  return result;
}

inline SizeF operator-(const SizeF& lhs, const SizeF& rhs) {
  SizeF result = lhs;
  result.Enlarge(-rhs.width(), -rhs.height());
  return result;
}

NATIVEUI_EXPORT SizeF ScaleSize(const SizeF& p, float x_scale, float y_scale);

inline SizeF ScaleSize(const SizeF& p, float scale) {
  return ScaleSize(p, scale, scale);
}

// This is declared here for use in gtest-based unit tests but is defined in
// the gfx_test_support target. Depend on that to use this in your unit test.
// This should not be used in production code - call ToString() instead.
void PrintTo(const SizeF& size, ::std::ostream* os);

}  // namespace nu

#endif  // NATIVEUI_GFX_GEOMETRY_SIZE_F_H_
