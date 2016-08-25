// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_GDIPLUS_H_
#define NATIVEUI_WIN_GDIPLUS_H_

#include <windows.h>

// Work around the compilation problems when using gdiplus.
#include <algorithm>
namespace Gdiplus {
  using std::max;
  using std::min;
}  // namespace Gdiplus

#include <gdiplus.h>  // NOLINT

#include "nativeui/graphics/color.h"
#include "ui/gfx/geometry/rect.h"

namespace nu {

// Convert from gfx types to gdiplus types.
inline Gdiplus::RectF ToGdi(const gfx::Rect& rect) {
  return Gdiplus::RectF(rect.x(), rect.y(), rect.width(), rect.height());
}

inline Gdiplus::PointF ToGdi(const gfx::Point& point) {
  return Gdiplus::PointF(point.x(), point.y());
}

inline Gdiplus::Color ToGdi(const Color& color) {
  return Gdiplus::Color(color.a(), color.r(), color.g(), color.b());
}

}  // namespace nu

#endif  // NATIVEUI_WIN_GDIPLUS_H_
