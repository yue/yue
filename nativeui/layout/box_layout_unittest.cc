// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class BoxLayoutTest : public testing::Test {
 protected:
  void SetUp() override {
    window_ = new nu::Window(nu::Window::Options());
    nu::Container* container = new nu::Container;
    container->SetLayoutManager(new nu::BoxLayout(nu::BoxLayout::Vertical));
    window_->SetContentView(container);
  }

  nu::State state_;
  scoped_refptr<nu::Window> window_;
};

TEST_F(BoxLayoutTest, ListChild) {
  nu::Rect bounds(1, 2, 300, 400);
  nu::Container* c1 = new nu::Container;
  nu::Container* c2 = new nu::Container;
  window_->GetContentView()->AddChildView(c1);
  window_->GetContentView()->AddChildView(c2);
  window_->SetContentBounds(bounds);
  EXPECT_EQ(c1->GetBounds(), nu::Rect(0, 0, 300, 200));
  EXPECT_EQ(c2->GetBounds(), nu::Rect(0, 200, 300, 200));
  EXPECT_EQ(c1->GetWindowOrigin(), nu::Point(0, 0));
  EXPECT_EQ(c2->GetWindowOrigin(), nu::Point(0, 200));
}

TEST_F(BoxLayoutTest, ListChildChild) {
  nu::Rect bounds(1, 2, 300, 400);
  nu::Container* c1 = new nu::Container;
  nu::Container* c2 = new nu::Container;
  c1->SetLayoutManager(new nu::BoxLayout(nu::BoxLayout::Vertical));
  c1->AddChildView(c2);
  window_->GetContentView()->AddChildView(c1);
  window_->SetContentBounds(bounds);
  EXPECT_EQ(c1->GetBounds(), nu::Rect(0, 0, 300, 400));
  EXPECT_EQ(c2->GetBounds(), nu::Rect(0, 0, 300, 400));
  EXPECT_EQ(c1->GetWindowOrigin(), nu::Point(0, 0));
  EXPECT_EQ(c2->GetWindowOrigin(), nu::Point(0, 0));
}

TEST_F(BoxLayoutTest, ListChildHorizontal) {
  nu::Rect bounds(1, 2, 300, 400);
  window_->GetContentView()->SetLayoutManager(
      new nu::BoxLayout(nu::BoxLayout::Horizontal));
  nu::Container* c1 = new nu::Container;
  nu::Container* c2 = new nu::Container;
  window_->GetContentView()->AddChildView(c1);
  window_->GetContentView()->AddChildView(c2);
  window_->SetContentBounds(bounds);
  EXPECT_EQ(c1->GetBounds(), nu::Rect(0, 0, 150, 400));
  EXPECT_EQ(c2->GetBounds(), nu::Rect(150, 0, 150, 400));
  EXPECT_EQ(c1->GetWindowOrigin(), nu::Point(0, 0));
  EXPECT_EQ(c2->GetWindowOrigin(), nu::Point(150, 0));
}
