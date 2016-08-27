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
  ASSERT_EQ(label_->GetText(), "test");
}
