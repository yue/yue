// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVEUI_GFX_GEOMETRY_VECTOR2D_CONVERSIONS_H_
#define NATIVEUI_GFX_GEOMETRY_VECTOR2D_CONVERSIONS_H_

#include "nativeui/gfx/geometry/vector2d.h"
#include "nativeui/gfx/geometry/vector2d_f.h"

namespace nu {

// Returns a Vector2d with each component from the input Vector2dF floored.
NATIVEUI_EXPORT Vector2d ToFlooredVector2d(const Vector2dF& vector2d);

// Returns a Vector2d with each component from the input Vector2dF ceiled.
NATIVEUI_EXPORT Vector2d ToCeiledVector2d(const Vector2dF& vector2d);

// Returns a Vector2d with each component from the input Vector2dF rounded.
NATIVEUI_EXPORT Vector2d ToRoundedVector2d(const Vector2dF& vector2d);

}  // namespace nu

#endif  // NATIVEUI_GFX_GEOMETRY_VECTOR2D_CONVERSIONS_H_
