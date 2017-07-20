// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class GroupTest : public testing::Test {
 protected:
  nu::Lifetime lifetime_;
  nu::State state_;
};

TEST_F(GroupTest, ContentView) {
  scoped_refptr<nu::Group> group(new nu::Group("title"));
  nu::Container* view = new nu::Container;
  group->SetContentView(view);
  EXPECT_EQ(group->GetContentView(), view);
}

TEST_F(GroupTest, Title) {
  scoped_refptr<nu::Group> group(new nu::Group("title"));
  group->SetTitle("test");
  EXPECT_EQ(group->GetTitle(), "test");
}

TEST_F(GroupTest, EmbeddingScroll) {
  scoped_refptr<nu::Group> group(new nu::Group("title"));
  group->SetContentView(new nu::Scroll);
}
