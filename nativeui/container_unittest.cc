// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/init.h"
#include "nativeui/label.h"
#include "nativeui/layout/fill_layout.h"
#include "nativeui/layout/box_layout.h"
#include "nativeui/window.h"
#include "testing/gtest/include/gtest/gtest.h"

class TestContainer : public nu::Container {
 public:
  TestContainer() {}

  void Layout() override {
    nu::Container::Layout();
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

TEST_F(ContainerTest, ChangeLayoutManager) {
  EXPECT_EQ(container_->layout_count(), 1);
  container_->SetLayoutManager(new nu::FillLayout);
  EXPECT_EQ(container_->layout_count(), 2);
}

TEST_F(ContainerTest, RemoveAndAddBack) {
  scoped_refptr<nu::Label> v = new nu::Label;
  container_->AddChildView(v.get());
  EXPECT_EQ(container_->child_count(), 1);
  container_->RemoveChildView(v.get());
  EXPECT_EQ(container_->child_count(), 0);
  container_->AddChildView(v.get());
  EXPECT_EQ(container_->child_count(), 1);
}

TEST_F(ContainerTest, MoveBetweenContainers) {
  scoped_refptr<nu::Label> v1 = new nu::Label;
  scoped_refptr<nu::Label> v2 = new nu::Label;
  nu::Container* c1 = new nu::Container;
  c1->SetLayoutManager(new nu::BoxLayout(nu::BoxLayout::Vertical));
  nu::Container* c2 = new nu::Container;
  c2->SetLayoutManager(new nu::BoxLayout(nu::BoxLayout::Horizontal));
  container_->SetLayoutManager(new nu::BoxLayout(nu::BoxLayout::Horizontal));
  c1->AddChildView(v1.get());
  c1->AddChildView(v2.get());
  container_->AddChildView(c1);
  container_->AddChildView(c2);
  container_->SetBounds(gfx::Rect(0, 0, 200, 400));
  EXPECT_EQ(v1->GetBounds(), gfx::Rect(0, 0, 100, 200));
  EXPECT_EQ(v2->GetBounds(), gfx::Rect(0, 200, 100, 200));

  c1->RemoveChildView(v1.get());
  c1->RemoveChildView(v2.get());
  c2->AddChildView(v1.get());
  c2->AddChildView(v2.get());
  EXPECT_EQ(v1->GetBounds(), gfx::Rect(0, 0, 50, 400));
  EXPECT_EQ(v2->GetBounds(), gfx::Rect(50, 0, 50, 400));
}
