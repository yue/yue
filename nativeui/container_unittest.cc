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
    window_->SetBounds(nu::RectF(0, 0, 400, 400));
    container_ = new TestContainer;
    window_->SetContentView(container_.get());
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::Window> window_;
  scoped_refptr<TestContainer> container_;
};

TEST_F(ContainerTest, AddChildView) {
  nu::Slider* v1 = new nu::Slider;
  nu::Slider* v2 = new nu::Slider;
  container_->AddChildView(v1);
  container_->AddChildView(v2);
  EXPECT_EQ(container_->ChildAt(0), v1);
  EXPECT_EQ(container_->ChildAt(1), v2);
}

TEST_F(ContainerTest, AddChildViewAt) {
  nu::Slider* v1 = new nu::Slider;
  nu::Slider* v2 = new nu::Slider;
  container_->AddChildView(v1);
  container_->AddChildViewAt(v2, 0);
  EXPECT_EQ(container_->ChildAt(0), v2);
  EXPECT_EQ(container_->ChildAt(1), v1);
}

TEST_F(ContainerTest, RemoveChildView) {
  nu::Slider* v1 = new nu::Slider;
  nu::Slider* v2 = new nu::Slider;
  container_->AddChildView(v1);
  container_->AddChildView(v2);
  container_->RemoveChildView(v1);
  EXPECT_EQ(container_->ChildAt(0), v2);
}

TEST_F(ContainerTest, SetBounds) {
  scoped_refptr<TestContainer> c = new TestContainer;
  EXPECT_EQ(c->layout_count(), 0);
  c->SetBounds(nu::RectF(0, 0, 100, 100));
  EXPECT_EQ(c->layout_count(), 1);
}

TEST_F(ContainerTest, Layout) {
  EXPECT_EQ(container_->layout_count(), 1);
  container_->AddChildView(new nu::Container);
  EXPECT_EQ(container_->layout_count(), 2);
  window_->SetBounds(nu::RectF(0, 0, 100, 200));
  EXPECT_EQ(container_->layout_count(), 3);
}

TEST_F(ContainerTest, ChildLayout) {
  window_->SetBounds(nu::RectF(0, 0, 100, 200));
  TestContainer* c1 = new TestContainer;
  container_->AddChildView(c1);
  EXPECT_EQ(c1->layout_count(), 0) << "Child CSS node should not layout";
}

TEST_F(ContainerTest, VisibleLayout) {
  window_->SetVisible(true);
  EXPECT_GE(container_->layout_count(), 1);
  container_->AddChildView(new nu::Container);
  EXPECT_GE(container_->layout_count(), 2);
  window_->SetBounds(nu::RectF(0, 0, 100, 200));
#if !defined(OS_LINUX)  // SetBounds is async in GTK+
  EXPECT_GE(container_->layout_count(), 3);
#endif
}

TEST_F(ContainerTest, RemoveAndAddBack) {
  scoped_refptr<nu::Slider> v = new nu::Slider;
  container_->AddChildView(v.get());
  EXPECT_EQ(container_->ChildCount(), 1);
  container_->RemoveChildView(v.get());
  EXPECT_EQ(container_->ChildCount(), 0);
  container_->AddChildView(v.get());
  EXPECT_EQ(container_->ChildCount(), 1);
}

TEST_F(ContainerTest, MoveBetweenContainers) {
  window_->SetContentSize(nu::SizeF(200, 400));

  nu::Container* c1 = new nu::Container;
  c1->SetStyle("flex", 1,
               "flex-direction", "row",
               "align-content", "stretch");
  container_->AddChildView(c1);

  nu::Container* c2 = new nu::Container;
  c2->SetStyle("flex", 1,
               "flex-direction", "column",
               "align-content", "stretch");
  container_->AddChildView(c2);

  scoped_refptr<nu::Container> v1 = new nu::Container;
  v1->SetStyle("flex", "1");
  scoped_refptr<nu::Container> v2 = new nu::Container;
  v2->SetStyle("flex", "1");
  c1->AddChildView(v1.get());
  c1->AddChildView(v2.get());
  EXPECT_EQ(v1->GetBounds(), nu::RectF(0, 0, 100, 200));
  EXPECT_EQ(v2->GetBounds(), nu::RectF(100, 0, 100, 200));

  c1->RemoveChildView(v1.get());
  c1->RemoveChildView(v2.get());
  c2->AddChildView(v1.get());
  c2->AddChildView(v2.get());
  EXPECT_EQ(v1->GetBounds(), nu::RectF(0, 0, 200, 100));
  EXPECT_EQ(v2->GetBounds(), nu::RectF(0, 100, 200, 100));
}
