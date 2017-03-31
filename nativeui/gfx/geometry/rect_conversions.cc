// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nativeui/gfx/geometry/rect_conversions.h"

#include <algorithm>
#include <cmath>

#include "base/logging.h"
#include "nativeui/gfx/geometry/safe_integer_conversions.h"

namespace nu {

Rect ToEnclosingRect(const RectF& rect) {
  int min_x = ToFlooredInt(rect.x());
  int min_y = ToFlooredInt(rect.y());
  float max_x = rect.right();
  float max_y = rect.bottom();
  int width =
      rect.width() == 0
          ? 0
          : std::max(
                ToCeiledInt(static_cast<double>(ToCeiledInt(max_x)) - min_x),
                0);
  int height =
      rect.height() == 0
          ? 0
          : std::max(
                ToCeiledInt(static_cast<double>(ToCeiledInt(max_y)) - min_y),
                0);
  return Rect(min_x, min_y, width, height);
}

Rect ToEnclosedRect(const RectF& rect) {
  int min_x = ToCeiledInt(rect.x());
  int min_y = ToCeiledInt(rect.y());
  float max_x = rect.right();
  float max_y = rect.bottom();
  int width = std::max(
      ToFlooredInt(static_cast<float>(ToFlooredInt(max_x)) - min_x), 0);
  int height = std::max(
      ToFlooredInt(static_cast<float>(ToFlooredInt(max_y)) - min_y), 0);
  return Rect(min_x, min_y, width, height);
}

Rect ToNearestRect(const RectF& rect) {
  float float_min_x = rect.x();
  float float_min_y = rect.y();
  float float_max_x = rect.right();
  float float_max_y = rect.bottom();

  int min_x = ToRoundedInt(float_min_x);
  int min_y = ToRoundedInt(float_min_y);
  int max_x = ToRoundedInt(float_max_x);
  int max_y = ToRoundedInt(float_max_y);

  return Rect(min_x, min_y, max_x - min_x, max_y - min_y);
}

bool IsNearestRectWithinDistance(const RectF& rect, float distance) {
  float float_min_x = rect.x();
  float float_min_y = rect.y();
  float float_max_x = rect.right();
  float float_max_y = rect.bottom();

  int min_x = ToRoundedInt(float_min_x);
  int min_y = ToRoundedInt(float_min_y);
  int max_x = ToRoundedInt(float_max_x);
  int max_y = ToRoundedInt(float_max_y);

  return
      (std::abs(min_x - float_min_x) < distance) &&
      (std::abs(min_y - float_min_y) < distance) &&
      (std::abs(max_x - float_max_x) < distance) &&
      (std::abs(max_y - float_max_y) < distance);
}

}  // namespace nu
