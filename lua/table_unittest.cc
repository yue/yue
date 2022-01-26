// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <string>

#include "lua/lua.h"
#include "testing/gtest/include/gtest/gtest.h"

class TableTest : public testing::Test {
 protected:
  void SetUp() override {
    lua::SetTop(state_, 0);
  }

  lua::ManagedState state_;
};

TEST_F(TableTest, RawSetGet) {
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

TEST_F(TableTest, RawGetWithPop) {
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

TEST_F(TableTest, RawGetWithPopNonExistKey) {
  lua::NewTable(state_);
  lua::RawSet(state_, 1, "key", "value");
  std::function<void()> f;
  EXPECT_FALSE(lua::RawGetAndPop(state_, 1, "non-exist", &f));
  std::string str;
  EXPECT_FALSE(lua::RawGetAndPop(state_, 1, "key", &str, "non-exist", &f));
}

TEST_F(TableTest, RawGetWithPopWrongType) {
  lua::NewTable(state_);
  lua::RawSet(state_, 1, "key", "value");
  int number;
  ASSERT_FALSE(lua::RawGetAndPop(state_, 1, "key", &number));
}

TEST_F(TableTest, ReadOptions) {
  lua::NewTable(state_);
  lua::RawSet(state_, 1, "key1", 123, "key2", "value");
  int number;
  std::string str;
  ASSERT_TRUE(lua::ReadOptions(state_, 1, "key1", &number, "key2", &str));
  EXPECT_EQ(number, 123);
  EXPECT_EQ(str, "value");
  EXPECT_EQ(lua::GetTop(state_), 1);
  bool b;
  ASSERT_FALSE(lua::RawGetAndPop(state_, 1, "key1", &b));
  ASSERT_EQ(lua::GetTop(state_), 1);
}

TEST_F(TableTest, ReadOptionsNonExistKey) {
  lua::NewTable(state_);
  lua::RawSet(state_, 1, "key", "value");
  std::function<void()> f;
  ASSERT_TRUE(lua::ReadOptions(state_, 1, "non-exist", &f));
  EXPECT_FALSE(f);
  std::string str;
  ASSERT_TRUE(lua::ReadOptions(state_, 1, "key", &str, "non-exist", &f));
  EXPECT_FALSE(f);
}

TEST_F(TableTest, ReadOptionsWrongType) {
  lua::NewTable(state_);
  lua::RawSet(state_, 1, "key", "value");
  int number;
  ASSERT_FALSE(lua::ReadOptions(state_, 1, "key", &number));
}

TEST_F(TableTest, PSetGet) {
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

TEST_F(TableTest, PSetValueOnStack) {
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

TEST_F(TableTest, PGetAndPop) {
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
