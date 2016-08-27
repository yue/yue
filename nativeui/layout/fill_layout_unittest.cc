// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class FillLayoutTest : public testing::Test {
 protected:
  void SetUp() override {
    window_ = new nu::Window(nu::Window::Options());
    nu::Container* container = new nu::Container;
    container->SetLayoutManager(new nu::FillLayout);
    window_->SetContentView(container);
  }

  nu::State state_;
  scoped_refptr<nu::Window> window_;
};

TEST_F(FillLayoutTest, FillChild) {
  nu::Rect bounds(1, 2, 300, 400);
  nu::Container* c1 = new nu::Container;
  nu::Container* c2 = new nu::Container;
  window_->GetContentView()->AddChildView(c1);
  window_->GetContentView()->AddChildView(c2);
  window_->SetContentBounds(bounds);
  EXPECT_EQ(c1->GetBounds(), nu::Rect(300, 400));
}

TEST_F(FillLayoutTest, FillChildChild) {
  nu::Rect bounds(1, 2, 300, 400);
  nu::Container* c1 = new nu::Container;
  nu::Container* c2 = new nu::Container;
  c1->AddChildView(c2);
  window_->GetContentView()->AddChildView(c1);
  window_->SetContentBounds(bounds);
  EXPECT_EQ(c1->GetBounds(), nu::Rect(300, 400));
  EXPECT_EQ(c2->GetBounds(), nu::Rect(300, 400));
}
