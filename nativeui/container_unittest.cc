// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"
#include "nativeui/init.h"
#include "nativeui/window.h"
#include "testing/gtest/include/gtest/gtest.h"

class ContainerTest : public testing::Test {
 protected:
  void SetUp() override {
    nu::Initialize();
    window_ = new nu::Window(nu::Window::Options());
  }

  scoped_refptr<nu::Window> window_;
};

class TestContainer : public nu::Container {
 public:
  TestContainer() {}

  void Layout() override {
    ++layout_count_;
  }

  int layout_count() const { return layout_count_; }

 private:
  ~TestContainer() override {}

  int layout_count_ = 0;
};

TEST_F(ContainerTest, Layout) {
  TestContainer* container = new TestContainer;
  EXPECT_EQ(container->layout_count(), 0);
  window_->SetContentView(container);
  EXPECT_EQ(container->layout_count(), 1);
  container->AddChildView(new nu::Container);
  EXPECT_EQ(container->layout_count(), 2);
  container->SetBounds(gfx::Rect(0, 0, 100, 200));
  EXPECT_EQ(container->layout_count(), 3);
}
