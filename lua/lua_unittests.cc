// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "lua/callback.h"
#include "lua/lua.h"
#include "lua/pop.h"
#include "testing/gtest/include/gtest/gtest.h"

class LuaTest : public testing::Test {
 protected:
  void SetUp() override {
    lua::SetTop(state_, 0);
  }

  lua::ManagedState state_;
};

TEST_F(LuaTest, PushesToStack) {
  ASSERT_TRUE(lua::Push(state_, 1));
  ASSERT_TRUE(lua::Push(state_, "str1"));
  ASSERT_TRUE(lua::Push(state_, "str2"));
  std::string str2;
  ASSERT_TRUE(lua::To(state_, -1, &str2));
  ASSERT_EQ(str2, "str2");
  base::StringPiece str1;
  ASSERT_TRUE(lua::To(state_, -2, &str1));
  ASSERT_EQ(str1, "str1");
  int number;
  ASSERT_TRUE(lua::To(state_, -3, &number));
  ASSERT_EQ(number, 1);
  ASSERT_EQ(lua::GetTop(state_), 3);
}

TEST_F(LuaTest, PushesAndGetsMultipleValues) {
  ASSERT_TRUE(lua::Push(state_, 1, 2, 3, 4, 5));
  int i1, i2, i3, i4, i5;
  ASSERT_TRUE(lua::To(state_, -5, &i1, &i2, &i3, &i4, &i5));
  EXPECT_EQ(i1, 1);
  EXPECT_EQ(i2, 2);
  EXPECT_EQ(i3, 3);
  EXPECT_EQ(i4, 4);
  EXPECT_EQ(i5, 5);
  ASSERT_EQ(lua::GetTop(state_), 5);
}

TEST_F(LuaTest, PopsValues) {
  ASSERT_TRUE(lua::Push(state_, 1, 2, 3, 4, 5));
  int i1, i2, i3, i4, i5;
  ASSERT_TRUE(lua::Pop(state_, &i1, &i2, &i3, &i4, &i5));
  EXPECT_EQ(i1, 1);
  EXPECT_EQ(i2, 2);
  EXPECT_EQ(i3, 3);
  EXPECT_EQ(i4, 4);
  EXPECT_EQ(i5, 5);
  ASSERT_EQ(lua::GetTop(state_), 0);
}

void FunctionWithoutReturnValue() {
}

void FunctionWithArgs(int, const std::string&) {
}

TEST_F(LuaTest, PCallWithoutReturnValue) {
  EXPECT_TRUE(lua::PCall(state_, base::Bind(&FunctionWithoutReturnValue)));
  ASSERT_EQ(lua::GetTop(state_), 0);
  EXPECT_TRUE(lua::PCall(state_, base::Bind(&FunctionWithArgs, 123, "test")));
  ASSERT_EQ(lua::GetTop(state_), 0);
}

int FunctionReturnsInt(int num) {
  return num;
}

std::string FunctionReturnsString(base::StringPiece str) {
  return std::string(str.data(), str.size());
}

TEST_F(LuaTest, PCallWithReturnValue) {
  int num = 42;
  int out = 0;
  ASSERT_TRUE(lua::PCall(state_, base::Bind(&FunctionReturnsInt), &out, num));
  EXPECT_EQ(num, out);
  ASSERT_EQ(lua::GetTop(state_), 0);
  base::StringPiece str = "valar morghulis";
  std::string str_out;
  ASSERT_TRUE(lua::PCall(state_, base::Bind(&FunctionReturnsString),
                         &str_out, str));
  EXPECT_EQ(str_out, str);
  ASSERT_EQ(lua::GetTop(state_), 0);
}
