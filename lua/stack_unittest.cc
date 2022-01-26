// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <string>
#include <tuple>

#include "lua/lua.h"
#include "testing/gtest/include/gtest/gtest.h"

class StackTest : public testing::Test {
 protected:
  void SetUp() override {
    lua::SetTop(state_, 0);
  }

  lua::ManagedState state_;
};

TEST_F(StackTest, PushesToStack) {
  lua::Push(state_, 1);
  lua::Push(state_, "str1");
  lua::Push(state_, "strr2");
  std::string str2;
  ASSERT_TRUE(lua::To(state_, -1, &str2));
  ASSERT_EQ(str2, "strr2");
  base::StringPiece str1;
  ASSERT_TRUE(lua::To(state_, -2, &str1));
  ASSERT_EQ(str1, "str1");
  double number;
  ASSERT_TRUE(lua::To(state_, -3, &number));
  ASSERT_FALSE(lua::To(state_, -1, &number));
  ASSERT_EQ(number, 1);
  ASSERT_EQ(lua::GetTop(state_), 3);
}

TEST_F(StackTest, PushesAndGetsMultipleValues) {
  lua::Push(state_, 1, 2, 3, 4, 5);
  int i1, i2, i3, i4, i5;
  ASSERT_TRUE(lua::To(state_, -5, &i1, &i2, &i3, &i4, &i5));
  EXPECT_EQ(i1, 1);
  EXPECT_EQ(i2, 2);
  EXPECT_EQ(i3, 3);
  EXPECT_EQ(i4, 4);
  EXPECT_EQ(i5, 5);
  ASSERT_EQ(lua::GetTop(state_), 5);
}

TEST_F(StackTest, PopsValues) {
  lua::Push(state_, 1, 2, 3, 4, 5);
  int i1, i2, i3, i4, i5;
  ASSERT_TRUE(lua::Pop(state_, &i1, &i2, &i3, &i4, &i5));
  EXPECT_EQ(i1, 1);
  EXPECT_EQ(i2, 2);
  EXPECT_EQ(i3, 3);
  EXPECT_EQ(i4, 4);
  EXPECT_EQ(i5, 5);
  ASSERT_EQ(lua::GetTop(state_), 0);
}

TEST_F(StackTest, TupleRepresentsMultipleValues) {
  std::tuple<int, int, int> rets;
  lua::Push(state_, 1, 2, 3, 4);
  ASSERT_TRUE(lua::Pop(state_, &rets));
  EXPECT_EQ(std::get<0>(rets), 2);
  EXPECT_EQ(std::get<1>(rets), 3);
  EXPECT_EQ(std::get<2>(rets), 4);
  int fourth;
  ASSERT_TRUE(lua::Pop(state_, &fourth));
  ASSERT_EQ(fourth, 1);
  ASSERT_EQ(lua::GetTop(state_), 0);
}
