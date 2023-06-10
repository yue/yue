// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class SliderTest : public testing::Test {
 protected:
  void SetUp() override {
    slider_ = new nu::Slider();
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::Slider> slider_;
};

TEST_F(SliderTest, SetValue) {
  slider_->SetValue(10);
  EXPECT_EQ(slider_->GetValue(), 10);
}

TEST_F(SliderTest, OutOfRange) {
  slider_->SetValue(10000);
  EXPECT_EQ(slider_->GetValue(), 100);
}

TEST_F(SliderTest, DefaultRange) {
  auto range = slider_->GetRange();
  EXPECT_EQ(std::get<0>(range), 0.);
  EXPECT_EQ(std::get<1>(range), 100.);
}

TEST_F(SliderTest, SetValueShouldNotEmitEvent) {
  bool emitted = false;
  slider_->on_value_change.Connect([&emitted](nu::Slider*) {
    emitted = true;
  });
  slider_->SetValue(10);
  EXPECT_FALSE(emitted);
}
