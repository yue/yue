// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
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
    window_ = new nu::Window(nu::Window::Options());
    container_ = new TestContainer;
    window_->SetContentView(container_.get());
  }

  nu::State state_;
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

TEST_F(ContainerTest, SetBounds) {
  scoped_refptr<TestContainer> c = new TestContainer;
  EXPECT_EQ(c->layout_count(), 0);
  c->SetBounds(nu::Rect(0, 0, 100, 100));
  EXPECT_EQ(c->layout_count(), 1);
}

TEST_F(ContainerTest, Layout) {
  EXPECT_EQ(container_->layout_count(), 1);
  container_->AddChildView(new nu::Container);
  EXPECT_EQ(container_->layout_count(), 2);
  window_->SetBounds(nu::Rect(0, 0, 100, 200));
  EXPECT_EQ(container_->layout_count(), 3);
}

TEST_F(ContainerTest, NestedLayout) {
  TestContainer* c1 = new TestContainer;
  window_->SetBounds(nu::Rect(0, 0, 100, 200));
  container_->AddChildView(c1);
  EXPECT_EQ(c1->layout_count(), 1);
}

TEST_F(ContainerTest, VisibleLayout) {
  window_->SetVisible(true);
  EXPECT_GE(container_->layout_count(), 1);
  container_->AddChildView(new nu::Container);
  EXPECT_GE(container_->layout_count(), 2);
  window_->SetBounds(nu::Rect(0, 0, 100, 200));
  EXPECT_GE(container_->layout_count(), 3);
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
  nu::BoxLayout* l1 = new nu::BoxLayout(nu::BoxLayout::Vertical);
  l1->set_main_axis_alignment(nu::BoxLayout::Stretch);
  l1->set_cross_axis_alignment(nu::BoxLayout::Stretch);
  c1->SetLayoutManager(l1);
  nu::Container* c2 = new nu::Container;
  nu::BoxLayout* l2 = new nu::BoxLayout(nu::BoxLayout::Horizontal);
  l2->set_main_axis_alignment(nu::BoxLayout::Stretch);
  l2->set_cross_axis_alignment(nu::BoxLayout::Stretch);
  c2->SetLayoutManager(l2);
  nu::BoxLayout* l3 = new nu::BoxLayout(nu::BoxLayout::Horizontal);
  l3->set_main_axis_alignment(nu::BoxLayout::Stretch);
  l3->set_cross_axis_alignment(nu::BoxLayout::Stretch);
  container_->SetLayoutManager(l3);

  c1->AddChildView(v1.get());
  c1->AddChildView(v2.get());
  container_->AddChildView(c1);
  container_->AddChildView(c2);
  window_->SetContentBounds(nu::Rect(0, 0, 200, 400));
  EXPECT_EQ(container_->GetBounds(), nu::Rect(0, 0, 200, 400));
  EXPECT_EQ(v1->GetBounds(), nu::Rect(0, 0, 100, 200));
  EXPECT_EQ(v2->GetBounds(), nu::Rect(0, 200, 100, 200));
  EXPECT_EQ(v1->GetWindowOrigin(), nu::Point(0, 0));
  EXPECT_EQ(v2->GetWindowOrigin(), nu::Point(0, 200));

  c1->RemoveChildView(v1.get());
  c1->RemoveChildView(v2.get());
  c2->AddChildView(v1.get());
  c2->AddChildView(v2.get());
  EXPECT_EQ(v1->GetBounds(), nu::Rect(0, 0, 50, 400));
  EXPECT_EQ(v2->GetBounds(), nu::Rect(50, 0, 50, 400));
  EXPECT_EQ(v1->GetWindowOrigin(), nu::Point(100, 0));
  EXPECT_EQ(v2->GetWindowOrigin(), nu::Point(150, 0));
}

TEST_F(ContainerTest, ChildSetPreferredSize) {
  nu::Container* child = new nu::Container;
  EXPECT_EQ(container_->layout_count(), 1);
  container_->AddChildView(child);
  EXPECT_EQ(container_->layout_count(), 2);
  nu::Size preferred_size(100, 100);
  child->SetPreferredSize(preferred_size);
  EXPECT_EQ(container_->layout_count(), 3);
  EXPECT_EQ(container_->preferred_size(), preferred_size);
}
