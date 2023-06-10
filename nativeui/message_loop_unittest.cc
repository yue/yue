// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class MessageLoopTest : public testing::Test {
 protected:
  void SetUp() override {
  }

  nu::Lifetime lifetime_;
  nu::State state_;
};

TEST_F(MessageLoopTest, PostTask) {
  nu::MessageLoop::PostTask([]() {
    nu::MessageLoop::Quit();
  });
  nu::MessageLoop::Run();
}
