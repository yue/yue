// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVEUI_GFX_GEOMETRY_RECT_CONVERSIONS_H_
#define NATIVEUI_GFX_GEOMETRY_RECT_CONVERSIONS_H_

#include "nativeui/gfx/geometry/rect.h"
#include "nativeui/gfx/geometry/rect_f.h"

namespace nu {

// Returns the smallest Rect that encloses the given RectF.
Rect ToEnclosingRect(const RectF& rect);

// Returns the largest Rect that is enclosed by the given RectF.
Rect ToEnclosedRect(const RectF& rect);

// Returns the Rect after snapping the corners of the RectF to an integer grid.
// This should only be used when the RectF you provide is expected to be an
// integer rect with floating point error. If it is an arbitrary RectF, then
// you should use a different method.
Rect ToNearestRect(const RectF& rect);

// Returns true if the Rect produced after snapping the corners of the RectF
// to an integer grid is withing |distance|.
bool IsNearestRectWithinDistance(const RectF& rect, float distance);

// Returns a Rect obtained by flooring the values of the given RectF.
// Please prefer the previous two functions in new code.
Rect ToFlooredRectDeprecated(const RectF& rect);

}  // namespace nu

#endif  // NATIVEUI_GFX_GEOMETRY_RECT_CONVERSIONS_H_
