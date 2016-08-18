// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/init.h"
#include "nativeui/window.h"
#include "testing/gtest/include/gtest/gtest.h"

class WindowTest : public testing::Test {
 protected:
  void SetUp() override {
    nu::Initialize();
  }
};

TEST_F(WindowTest, SetVisible) {
  nu::Window::Options options;
  scoped_refptr<nu::Window> window(new nu::Window(options));
  window->SetVisible(true);
  ASSERT_TRUE(window->IsVisible());
  window->SetVisible(false);
  ASSERT_FALSE(window->IsVisible());
}
