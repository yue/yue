// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "base/bind.h"
#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class WindowTest : public testing::Test {
 protected:
  void SetUp() override {
    window_ = new nu::Window(nu::Window::Options());
  }

  nu::State state_;
  scoped_refptr<nu::Window> window_;
};

TEST_F(WindowTest, Bounds) {
  nu::RectF bounds(123, 456, 789, 10);
  window_->SetContentBounds(bounds);
  nu::RectF window_bounds = window_->GetBounds();
  EXPECT_EQ(window_->GetContentBounds(), bounds);
  window_->SetBounds(window_bounds);
  EXPECT_EQ(window_->GetBounds(), window_bounds);
}

TEST_F(WindowTest, ContentView) {
  EXPECT_NE(window_->GetContentView(), nullptr);
  scoped_refptr<nu::Container> view(new nu::Container);
  window_->SetContentView(view.get());
  EXPECT_EQ(window_->GetContentView(), view.get());
}

TEST_F(WindowTest, Visible) {
  window_->SetVisible(true);
  ASSERT_TRUE(window_->IsVisible());
  window_->SetVisible(false);
  ASSERT_FALSE(window_->IsVisible());
}

void OnClose(bool* ptr) {
  *ptr = true;
}

TEST_F(WindowTest, OnClose) {
  bool closed = false;
  window_->on_close.Connect(base::Bind(&OnClose, &closed));
  window_->Close();
  EXPECT_EQ(closed, true);
}

bool ShouldClose() {
  return false;
}

TEST_F(WindowTest, ShouldClose) {
  bool closed = false;
  window_->on_close.Connect(base::Bind(&OnClose, &closed));
  window_->should_close = base::Bind(&ShouldClose);
  window_->Close();
  EXPECT_EQ(closed, false);
  window_->should_close.Reset();
  window_->Close();
  EXPECT_EQ(closed, true);
}
