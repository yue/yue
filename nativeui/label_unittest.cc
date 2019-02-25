// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class LabelTest : public testing::Test {
 protected:
  void SetUp() override {
    label_ = new nu::Label("");
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::Label> label_;
};

TEST_F(LabelTest, SetText) {
  label_->SetText("test");
  EXPECT_EQ(label_->GetText(), "test");
}

// FIXME: Enable this test after we have View::GetStyle.
#if 0
TEST_F(LabelTest, UpdateStyle) {
  label_->SetText("test");
  YGValue width = YGNodeStyleGetMinWidth(label_->node());
  YGValue height = YGNodeStyleGetMinHeight(label_->node());
  label_->SetText("longlongtest");
  EXPECT_LT(width.value, YGNodeStyleGetMinWidth(label_->node()).value);
  EXPECT_EQ(height.value, YGNodeStyleGetMinHeight(label_->node()).value);
}
#endif
