// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVEUI_TEST_GFX_UTIL_H_
#define NATIVEUI_TEST_GFX_UTIL_H_

#include <iosfwd>
#include <string>

#include "nativeui/gfx/geometry/rect_f.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace nu {

#define EXPECT_RECTF_EQ(a, b) \
  EXPECT_PRED_FORMAT2(::gfx::AssertRectFloatEqual, a, b)

::testing::AssertionResult AssertRectFloatEqual(const char* lhs_expr,
                                                const char* rhs_expr,
                                                const RectF& lhs,
                                                const RectF& rhs);

}  // namespace nu

#endif  // NATIVEUI_TEST_GFX_UTIL_H_
