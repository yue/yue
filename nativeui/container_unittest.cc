// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/init.h"
#include "nativeui/label.h"
#include "nativeui/window.h"
#include "testing/gtest/include/gtest/gtest.h"

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

class ContainerTest : public testing::Test {
 protected:
  void SetUp() override {
    nu::Initialize();
    window_ = new nu::Window(nu::Window::Options());
    container_ = new TestContainer;
    window_->SetContentView(container_.get());
  }

  scoped_refptr<nu::Window> window_;
  scoped_refptr<TestContainer> container_;
};

TEST_F(ContainerTest, AddChildView) {
  nu::Label* v1 = new nu::Label;
  nu::Label* v2 = new nu::Label;
  container_->AddChildView(v1);
  container_->AddChildView(v2);
  EXPECT_EQ(container_->child_at(0), v1);
  EXPECT_EQ(container_->child_at(1), v2);
}

TEST_F(ContainerTest, AddChildViewAt) {
  nu::Label* v1 = new nu::Label;
  nu::Label* v2 = new nu::Label;
  container_->AddChildView(v1);
  container_->AddChildViewAt(v2, 0);
  EXPECT_EQ(container_->child_at(0), v2);
  EXPECT_EQ(container_->child_at(1), v1);
}

TEST_F(ContainerTest, RemoveChildView) {
  nu::Label* v1 = new nu::Label;
  nu::Label* v2 = new nu::Label;
  container_->AddChildView(v1);
  container_->AddChildView(v2);
  container_->RemoveChildView(v1);
  EXPECT_EQ(container_->child_at(0), v2);
}

TEST_F(ContainerTest, Layout) {
  EXPECT_EQ(container_->layout_count(), 1);
  container_->AddChildView(new nu::Container);
  EXPECT_EQ(container_->layout_count(), 2);
  container_->SetBounds(gfx::Rect(0, 0, 100, 200));
  EXPECT_EQ(container_->layout_count(), 3);
}
