// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <string>
#include <tuple>

#include "lua/lua.h"
#include "testing/gtest/include/gtest/gtest.h"

class LuaTest : public testing::Test {
 protected:
  void SetUp() override {
    lua::SetTop(state_, 0);
  }

  lua::ManagedState state_;
};

TEST_F(LuaTest, PushesToStack) {
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

TEST_F(LuaTest, PushesAndGetsMultipleValues) {
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

TEST_F(LuaTest, PopsValues) {
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

TEST_F(LuaTest, TupleRepresentsMultipleValues) {
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

TEST_F(LuaTest, RawSetGet) {
  lua::NewTable(state_);
  ASSERT_EQ(lua::GetTop(state_), 1);
  lua::RawSet(state_, 1, "key1", 1);
  lua::RawSet(state_, -1, "key2", 2, 1, "v1");
  lua::RawSet(state_, -1, "key3", 3, 2, "v2", 3, "v3");
  ASSERT_EQ(lua::GetTop(state_), 1);
  lua::RawGet(state_, -1, "key1", "key2", "key3", 1, 2, 3);
  int i1, i2, i3;
  std::string v1, v2, v3;
  ASSERT_TRUE(lua::Pop(state_, &i1, &i2, &i3, &v1, &v2, &v3));
  EXPECT_EQ(i1, 1);
  EXPECT_EQ(i2, 2);
  EXPECT_EQ(i3, 3);
  EXPECT_EQ(v1, "v1");
  EXPECT_EQ(v2, "v2");
  EXPECT_EQ(v3, "v3");
  lua::RawGet(state_, -1, "non-exist");
  EXPECT_EQ(lua::GetType(state_, -1), lua::LuaType::Nil);
}

TEST_F(LuaTest, RawGetWithPop) {
  lua::NewTable(state_);
  lua::RawSet(state_, 1, 123, "oldvalue");
  lua::RawSet(state_, 1, "key", 123, 123, "value");
  std::string str;
  int number;
  ASSERT_TRUE(lua::RawGetAndPop(state_, 1, "key", &str, "key", &number));
  EXPECT_EQ(str, "123");
  EXPECT_EQ(number, 123);
  ASSERT_TRUE(lua::RawGetAndPop(state_, 1, 123, &str));
  EXPECT_EQ(str, "value");
  EXPECT_EQ(lua::GetTop(state_), 1);
  bool b;
  ASSERT_FALSE(lua::RawGetAndPop(state_, 1, "key", &b));
  ASSERT_EQ(lua::GetTop(state_), 1);
}

TEST_F(LuaTest, RawGetWithPopNonExistKey) {
  lua::NewTable(state_);
  std::function<void()> f;
  ASSERT_FALSE(lua::RawGetAndPop(state_, 1, "key", &f));
}

TEST_F(LuaTest, PSetGet) {
  lua::NewTable(state_);
  ASSERT_EQ(lua::GetTop(state_), 1);
  ASSERT_TRUE(lua::PSet(state_, 1, "key1", 1, "key2", "value2", 3, "value3"));
  int v1;
  std::string v2, v3;
  ASSERT_TRUE(lua::PGet(state_, 1, "key1", "key2", 3));
  ASSERT_EQ(lua::GetTop(state_), 4);
  ASSERT_TRUE(lua::Pop(state_, &v1, &v2, &v3));
  EXPECT_EQ(v1, 1);
  EXPECT_EQ(v2, "value2");
  EXPECT_EQ(v3, "value3");
}

TEST_F(LuaTest, PSetValueOnStack) {
  lua::NewTable(state_);
  lua::Push(state_, 123);
  lua::Push(state_, "value");
  ASSERT_TRUE(lua::PSet(state_, 1,
                        "key1", lua::ValueOnStack(state_, 2),
                        "key2", lua::ValueOnStack(state_, 3)));
  int value1;
  std::string value2;
  ASSERT_TRUE(lua::PGetAndPop(state_, 1, "key1", &value1, "key2", &value2));
  EXPECT_EQ(value1, 123);
  EXPECT_EQ(value2, "value");
}

TEST_F(LuaTest, PGetAndPop) {
  lua::NewTable(state_);
  ASSERT_EQ(lua::GetTop(state_), 1);
  ASSERT_TRUE(lua::PSet(state_, 1, "key1", true, "key2", "v2", 3, "value3"));
  bool v1;
  std::string v2, v3;
  ASSERT_TRUE(lua::PGetAndPop(state_, 1, "key1", &v1, "key2", &v2, 3, &v3));
  ASSERT_EQ(lua::GetTop(state_), 1);
  EXPECT_EQ(v1, true);
  EXPECT_EQ(v2, "v2");
  EXPECT_EQ(v3, "value3");
  ASSERT_FALSE(lua::PGetAndPop(state_, 1, "key1", &v2));
  std::string error;
  ASSERT_TRUE(lua::Pop(state_, &error));
  EXPECT_EQ(error, "error converting values");
}
