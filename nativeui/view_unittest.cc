// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class ViewTest : public testing::Test {
 protected:
  void SetUp() override {
    view_ = new nu::Label("some text");
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::View> view_;
};

TEST_F(ViewTest, ClassName) {
  EXPECT_STREQ(view_->GetClassName(), "Label");
  EXPECT_EQ(view_->GetClassName(), nu::Label::kClassName);
}

TEST_F(ViewTest, Bounds) {
  nu::RectF bounds(100, 100, 200, 200);
  view_->SetBounds(bounds);
  EXPECT_EQ(view_->GetBounds(), bounds);
}

TEST_F(ViewTest, SetContentView) {
  scoped_refptr<nu::Window> window(new nu::Window(nu::Window::Options()));
  window->SetContentView(view_.get());
  window->SetContentSize(nu::SizeF(200, 200));
  EXPECT_EQ(view_->GetBounds(), nu::RectF(0, 0, 200, 200));
}

TEST_F(ViewTest, SetVisible) {
  EXPECT_EQ(view_->IsVisible(), true);
  view_->SetVisible(false);
  EXPECT_EQ(view_->IsVisible(), false);
}

TEST_F(ViewTest, HiddenViewSkipsLayout) {
  scoped_refptr<nu::Container> container(new nu::Container);
  container->AddChildView(view_.get());
  container->AddChildView(new nu::Label("l1"));
  EXPECT_GT(container->ChildAt(1)->GetBounds().y(), 0);
  view_->SetVisible(false);
  EXPECT_EQ(container->ChildAt(1)->GetBounds().y(), 0);
}

TEST_F(ViewTest, OnSizeChanged) {
  scoped_refptr<nu::Window> window(new nu::Window(nu::Window::Options()));
  window->SetContentView(view_.get());
  bool changed = false;
  view_->on_size_changed.Connect([&changed](nu::View*) {
    changed = true;
  });
  window->SetContentSize(nu::SizeF(100, 100));
  EXPECT_TRUE(changed);
}
