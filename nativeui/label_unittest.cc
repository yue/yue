// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/init.h"
#include "nativeui/label.h"
#include "testing/gtest/include/gtest/gtest.h"

class LabelTest : public testing::Test {
 protected:
  void SetUp() override {
    nu::Initialize();
  }
};

TEST_F(LabelTest, SetText) {
  scoped_refptr<nu::Label> label(new nu::Label);
  label->SetText("test");
  ASSERT_EQ(label->GetText(), "test");
}
