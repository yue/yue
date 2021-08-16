// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nativeui/gfx/geometry/size_f.h"

#if defined(OS_MAC)
#include <ApplicationServices/ApplicationServices.h>
#endif

#include "base/strings/stringprintf.h"

namespace nu {

#if defined(OS_MAC)
SizeF::SizeF(const CGSize& s)
    : width_(s.width < 0 ? 0 : s.width),
      height_(s.height < 0 ? 0 : s.height) {
}

SizeF& SizeF::operator=(const CGSize& s) {
  set_width(s.width);
  set_height(s.height);
  return *this;
}

CGSize SizeF::ToCGSize() const {
  return CGSizeMake(width(), height());
}
#endif

float SizeF::GetArea() const {
  return width() * height();
}

void SizeF::Enlarge(float grow_width, float grow_height) {
  SetSize(width() + grow_width, height() + grow_height);
}

void SizeF::SetToMin(const SizeF& other) {
  width_ = width() <= other.width() ? width() : other.width();
  height_ = height() <= other.height() ? height() : other.height();
}

void SizeF::SetToMax(const SizeF& other) {
  width_ = width() >= other.width() ? width() : other.width();
  height_ = height() >= other.height() ? height() : other.height();
}

std::string SizeF::ToString() const {
  return base::StringPrintf("%fx%f", width(), height());
}

SizeF ScaleSize(const SizeF& s, float x_scale, float y_scale) {
  SizeF scaled_s(s);
  scaled_s.Scale(x_scale, y_scale);
  return scaled_s;
}

}  // namespace nu
