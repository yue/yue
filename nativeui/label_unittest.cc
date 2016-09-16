// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/css-layout/CSSLayout/CSSLayout.h"

class LabelTest : public testing::Test {
 protected:
  void SetUp() override {
    label_ = new nu::Label;
  }

  nu::State state_;
  scoped_refptr<nu::Label> label_;
};

TEST_F(LabelTest, SetText) {
  label_->SetText("test");
  EXPECT_EQ(label_->GetText(), "test");
}

TEST_F(LabelTest, UpdateStyle) {
  label_->SetText("test");
  float width = CSSNodeStyleGetMinWidth(label_->node());
  float height = CSSNodeStyleGetMinHeight(label_->node());
  label_->SetText("longlongtest");
  EXPECT_LT(width, CSSNodeStyleGetMinWidth(label_->node()));
  EXPECT_EQ(height, CSSNodeStyleGetMinHeight(label_->node()));
}
