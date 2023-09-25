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

TEST_F(MessageLoopTest, PostDelayedTask) {
  nu::MessageLoop::PostDelayedTask(10, []() {
    nu::MessageLoop::Quit();
  });
  nu::MessageLoop::Run();
}

TEST_F(MessageLoopTest, SetTimer) {
  int count = 0;
  nu::MessageLoop::SetTimer(10, [&count]() {
    if (++count == 3) {
      nu::MessageLoop::Quit();
      return false;
    } else {
      return true;
    }
  });
  nu::MessageLoop::Run();
  ASSERT_EQ(count, 3);
}
