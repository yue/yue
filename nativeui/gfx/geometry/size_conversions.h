// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVEUI_GFX_GEOMETRY_SIZE_CONVERSIONS_H_
#define NATIVEUI_GFX_GEOMETRY_SIZE_CONVERSIONS_H_

#include "nativeui/gfx/geometry/size.h"
#include "nativeui/gfx/geometry/size_f.h"

namespace nu {

// Returns a Size with each component from the input SizeF floored.
NATIVEUI_EXPORT Size ToFlooredSize(const SizeF& size);

// Returns a Size with each component from the input SizeF ceiled.
NATIVEUI_EXPORT Size ToCeiledSize(const SizeF& size);

// Returns a Size with each component from the input SizeF rounded.
NATIVEUI_EXPORT Size ToRoundedSize(const SizeF& size);

}  // namespace nu

#endif  // NATIVEUI_GFX_GEOMETRY_SIZE_CONVERSIONS_H_
