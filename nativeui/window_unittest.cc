// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/init.h"
#include "nativeui/label.h"
#include "nativeui/window.h"
#include "testing/gtest/include/gtest/gtest.h"

class WindowTest : public testing::Test {
 protected:
  void SetUp() override {
    nu::Initialize();
    window_ = new nu::Window(nu::Window::Options());
  }

  scoped_refptr<nu::Window> window_;
};

TEST_F(WindowTest, ContentView) {
  EXPECT_EQ(window_->GetContentView(), nullptr);
  scoped_refptr<nu::View> view(new nu::Label);
  window_->SetContentView(view.get());
  EXPECT_EQ(window_->GetContentView(), view.get());
}

TEST_F(WindowTest, Visible) {
  window_->SetVisible(true);
  ASSERT_TRUE(window_->IsVisible());
  window_->SetVisible(false);
  ASSERT_FALSE(window_->IsVisible());
}
