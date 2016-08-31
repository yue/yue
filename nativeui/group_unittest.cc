// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class GroupTest : public testing::Test {
 protected:
  nu::State state_;
};

TEST_F(GroupTest, ContentView) {
  scoped_refptr<nu::Group> group(new nu::Group("title"));
  nu::Container* view = new nu::Container;
  group->SetContentView(view);
  EXPECT_EQ(group->GetContentView(), view);
  EXPECT_NE(view->GetWindowPixelOrigin(), nu::Point(0, 0));

  scoped_refptr<nu::Window> window(new nu::Window(nu::Window::Options()));
  window->SetBounds(nu::Rect(0, 0, 100, 100));
  window->GetContentView()->AddChildView(group.get());
  nu::Point gpos = group->GetWindowPixelOrigin();
  nu::Point vpos = view->GetWindowPixelOrigin();
  EXPECT_LT(gpos.x(), vpos.x());
  EXPECT_LT(gpos.y(), vpos.y());
}

TEST_F(GroupTest, Title) {
  scoped_refptr<nu::Group> group(new nu::Group("title"));
  group->SetTitle("test");
  ASSERT_EQ(group->GetTitle(), "test");
}
