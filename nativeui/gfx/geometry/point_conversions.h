// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVEUI_GFX_GEOMETRY_POINT_CONVERSIONS_H_
#define NATIVEUI_GFX_GEOMETRY_POINT_CONVERSIONS_H_

#include "nativeui/gfx/geometry/point.h"
#include "nativeui/gfx/geometry/point_f.h"

namespace nu {

// Returns a Point with each component from the input PointF floored.
NATIVEUI_EXPORT Point ToFlooredPoint(const PointF& point);

// Returns a Point with each component from the input PointF ceiled.
NATIVEUI_EXPORT Point ToCeiledPoint(const PointF& point);

// Returns a Point with each component from the input PointF rounded.
NATIVEUI_EXPORT Point ToRoundedPoint(const PointF& point);

}  // namespace nu

#endif  // NATIVEUI_GFX_GEOMETRY_POINT_CONVERSIONS_H_
