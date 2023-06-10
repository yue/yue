// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class ScreenTest : public testing::Test {
 protected:
  void SetUp() override {
    screen_ = nu::Screen::GetCurrent();
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  nu::Screen* screen_;
};

TEST_F(ScreenTest, GetPrimaryDisplay) {
  nu::Display display = screen_->GetPrimaryDisplay();
  EXPECT_NE(display.id, 0u);
  EXPECT_GE(display.scale_factor, 1.f);
  EXPECT_FALSE(display.bounds.IsEmpty());
  EXPECT_FALSE(display.work_area.IsEmpty());
#if defined(OS_MAC) || defined(OS_WIN)
  EXPECT_TRUE(display.work_area.height() < display.bounds.height() ||
              display.work_area.width() < display.bounds.width());
#endif
}

TEST_F(ScreenTest, GetAllDisplays) {
  nu::Display primary = screen_->GetPrimaryDisplay();
  std::vector<nu::Display> displays = screen_->GetAllDisplays();
  bool include_primary = false;
  for (const auto& display : displays) {
    if (display.id == primary.id) {
      include_primary = true;
      EXPECT_EQ(display.scale_factor, primary.scale_factor);
      EXPECT_EQ(display.bounds, primary.bounds);
      EXPECT_EQ(display.work_area, primary.work_area);
      break;
    }
  }
  EXPECT_TRUE(include_primary);
}

TEST_F(ScreenTest, GetDisplayNearestPoint) {
  nu::Display primary = screen_->GetPrimaryDisplay();
  nu::Display display = screen_->GetDisplayNearestPoint(nu::PointF(0, 0));
  EXPECT_EQ(primary.id, display.id);
}
