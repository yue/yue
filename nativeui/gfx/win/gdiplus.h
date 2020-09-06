// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_GDIPLUS_H_
#define NATIVEUI_GFX_WIN_GDIPLUS_H_

#include <windows.h>
#include <objidl.h>  // NOLINT

// Work around the compilation problems when using gdiplus.
#include <algorithm>
namespace Gdiplus {
  using std::max;
  using std::min;
}  // namespace Gdiplus

#include <gdiplus.h>  // NOLINT

#include "base/notreached.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/geometry/rect.h"
#include "nativeui/gfx/geometry/rect_f.h"
#include "nativeui/gfx/text.h"

namespace nu {

// Convert from gfx types to gdiplus types.
inline Gdiplus::RectF ToGdi(const RectF& rect) {
  return Gdiplus::RectF(rect.x(), rect.y(), rect.width(), rect.height());
}

inline Gdiplus::Rect ToGdi(const Rect& rect) {
  return Gdiplus::Rect(rect.x(), rect.y(), rect.width(), rect.height());
}

inline Gdiplus::PointF ToGdi(const PointF& point) {
  return Gdiplus::PointF(point.x(), point.y());
}

inline Gdiplus::Point ToGdi(const Point& point) {
  return Gdiplus::Point(point.x(), point.y());
}

inline Gdiplus::Color ToGdi(const Color& color) {
  return Gdiplus::Color(color.a(), color.r(), color.g(), color.b());
}

inline Gdiplus::StringAlignment ToGdi(TextAlign align) {
  switch (align) {
    case TextAlign::Start: return Gdiplus::StringAlignmentNear;
    case TextAlign::Center: return Gdiplus::StringAlignmentCenter;
    case TextAlign::End: return Gdiplus::StringAlignmentFar;
  }
  NOTREACHED();
  return Gdiplus::StringAlignmentNear;
}

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_GDIPLUS_H_
