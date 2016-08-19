// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/init.h"
#include "nativeui/label.h"
#include "nativeui/window.h"
#include "testing/gtest/include/gtest/gtest.h"

class ViewTest : public testing::Test {
 protected:
  void SetUp() override {
    nu::Initialize();
    view_ = new nu::Label;
  }

  scoped_refptr<nu::View> view_;
};

TEST_F(ViewTest, AddChildView) {
  nu::Label* v1 = new nu::Label;
  nu::Label* v2 = new nu::Label;
  view_->AddChildView(v1);
  view_->AddChildView(v2);
  EXPECT_EQ(view_->child_at(0), v1);
  EXPECT_EQ(view_->child_at(1), v2);
}

TEST_F(ViewTest, AddChildViewAt) {
  nu::Label* v1 = new nu::Label;
  nu::Label* v2 = new nu::Label;
  view_->AddChildView(v1);
  view_->AddChildViewAt(v2, 0);
  EXPECT_EQ(view_->child_at(0), v2);
  EXPECT_EQ(view_->child_at(1), v1);
}

TEST_F(ViewTest, RemoveChildView) {
  nu::Label* v1 = new nu::Label;
  nu::Label* v2 = new nu::Label;
  view_->AddChildView(v1);
  view_->AddChildView(v2);
  view_->RemoveChildView(v1);
  EXPECT_EQ(view_->child_at(0), v2);
}

TEST_F(ViewTest, Bounds) {
  gfx::Rect bounds(100, 100, 200, 200);
  view_->SetBounds(bounds);
  EXPECT_EQ(view_->GetBounds(), bounds);
}
