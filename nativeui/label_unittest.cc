// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

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

TEST_F(LabelTest, PreferredSize) {
  nu::Size preferred_size = label_->preferred_size();
  label_->SetText("test");
  EXPECT_NE(preferred_size, label_->preferred_size());
  preferred_size = label_->preferred_size();
  label_->SetText("longlongtest");
  EXPECT_NE(preferred_size, label_->preferred_size());
}
