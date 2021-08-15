// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVEUI_GFX_GEOMETRY_SAFE_INTEGER_CONVERSIONS_H_
#define NATIVEUI_GFX_GEOMETRY_SAFE_INTEGER_CONVERSIONS_H_

#include <cmath>

#include "base/numerics/safe_conversions.h"
#include "nativeui/nativeui_export.h"

namespace nu {

inline int ToFlooredInt(float value) {
  return base::saturated_cast<int>(std::floor(value));
}

inline int ToCeiledInt(float value) {
  return base::saturated_cast<int>(std::ceil(value));
}

inline int ToFlooredInt(double value) {
  return base::saturated_cast<int>(std::floor(value));
}

inline int ToCeiledInt(double value) {
  return base::saturated_cast<int>(std::ceil(value));
}

inline int ToRoundedInt(float value) {
  float rounded;
  if (value >= 0.0f)
    rounded = std::floor(value + 0.5f);
  else
    rounded = std::ceil(value - 0.5f);
  return base::saturated_cast<int>(rounded);
}

inline int ToRoundedInt(double value) {
  double rounded;
  if (value >= 0.0)
    rounded = std::floor(value + 0.5);
  else
    rounded = std::ceil(value - 0.5);
  return base::saturated_cast<int>(rounded);
}

}  // namespace nu

#endif  // NATIVEUI_GFX_GEOMETRY_SAFE_INTEGER_CONVERSIONS_H_
