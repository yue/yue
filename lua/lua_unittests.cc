// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <memory>

#include "base/bind.h"
#include "lua/handle.h"
#include "lua/pcall.h"
#include "lua/table.h"
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

TEST_F(LuaTest, PCallWithInvalidValue) {
  std::string str;
  lua::Push(state_, nullptr);
  EXPECT_FALSE(lua::PCall(state_, nullptr));
  ASSERT_EQ(lua::GetTop(state_), 1);
  ASSERT_TRUE(lua::Pop(state_, &str));
  ASSERT_EQ(str, "attempt to call a nil value");
}

void FunctionWithArgs(int, const std::string&) {
}

TEST_F(LuaTest, PCallWithInsufficientArgs) {
  std::string str;
  lua::Push(state_, base::Bind(&FunctionWithArgs));
  EXPECT_FALSE(lua::PCall(state_, nullptr, 123));
  ASSERT_TRUE(lua::Pop(state_, &str));
  ASSERT_EQ(str, "insufficient args, expecting 2 but got 1");
  ASSERT_EQ(lua::GetTop(state_), 0);

  lua::Push(state_, base::Bind(&FunctionWithArgs));
  EXPECT_FALSE(lua::PCall(state_, nullptr));
  ASSERT_TRUE(lua::Pop(state_, &str));
  ASSERT_EQ(str, "insufficient args, expecting 2 but got 0");
  ASSERT_EQ(lua::GetTop(state_), 0);
}

TEST_F(LuaTest, PCallWithWrongArgs) {
  std::string str;
  lua::Push(state_, base::Bind(&FunctionWithArgs));
  EXPECT_FALSE(lua::PCall(state_, nullptr, "test", 123));
  ASSERT_TRUE(lua::Pop(state_, &str));
  ASSERT_EQ(str, "error converting arg at index 1 from string to integer");
  ASSERT_EQ(lua::GetTop(state_), 0);
}

void FunctionWithoutReturnValue() {
}

TEST_F(LuaTest, PCallWithoutReturnValue) {
  lua::Push(state_, base::Bind(&FunctionWithoutReturnValue));
  EXPECT_TRUE(lua::PCall(state_, nullptr));
  ASSERT_EQ(lua::GetTop(state_), 0);
  lua::Push(state_, base::Bind(&FunctionWithArgs));
  EXPECT_TRUE(lua::PCall(state_, nullptr, 123, "test"));
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
  lua::Push(state_, base::Bind(&FunctionReturnsInt));
  ASSERT_TRUE(lua::PCall(state_, &out, num));
  EXPECT_EQ(num, out);
  ASSERT_EQ(lua::GetTop(state_), 0);

  base::StringPiece str = "valar morghulis";
  std::string str_out;
  lua::Push(state_, base::Bind(&FunctionReturnsString));
  ASSERT_TRUE(lua::PCall(state_, &str_out, str));
  EXPECT_EQ(str_out, str);
  ASSERT_EQ(lua::GetTop(state_), 0);
}

std::tuple<std::string, int> FunctionReturnsTuple(
    const std::string& str, int number) {
  return std::make_tuple(str, number);
}

TEST_F(LuaTest, PCallWithMultipleReturnValues) {
  lua::Push(state_, base::Bind(&FunctionReturnsTuple));
  std::tuple<std::string, int> out;
  ASSERT_TRUE(lua::PCall(state_, &out, "str", 123));
  EXPECT_EQ(std::get<0>(out), "str");
  EXPECT_EQ(std::get<1>(out), 123);
  ASSERT_EQ(lua::GetTop(state_), 0);
}

TEST_F(LuaTest, RawSetGet) {
  lua::PushNewTable(state_);
  ASSERT_EQ(lua::GetTop(state_), 1);
  lua::RawSet(state_, 1, "key1", 1);
  lua::RawSet(state_, 1, "key2", 2, 1, "v1");
  lua::RawSet(state_, 1, "key3", 3, 2, "v2", 3, "v3");
  ASSERT_EQ(lua::GetTop(state_), 1);
  lua::RawGet(state_, 1, "key1", "key2", "key3", 1, 2, 3);
  int i1, i2, i3;
  std::string v1, v2, v3;
  ASSERT_TRUE(lua::Pop(state_, &i1, &i2, &i3, &v1, &v2, &v3));
  EXPECT_EQ(i1, 1);
  EXPECT_EQ(i2, 2);
  EXPECT_EQ(i3, 3);
  EXPECT_EQ(v1, "v1");
  EXPECT_EQ(v2, "v2");
  EXPECT_EQ(v3, "v3");
  lua::RawGet(state_, 1, "non-exist");
  EXPECT_EQ(lua::GetType(state_, -1), lua::LuaType::Nil);
}

TEST_F(LuaTest, RawGetWithPop) {
  lua::PushNewTable(state_);
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

TEST_F(LuaTest, PSetGet) {
  lua::PushNewTable(state_);
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

TEST_F(LuaTest, PGetAndPop) {
  lua::PushNewTable(state_);
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

TEST_F(LuaTest, Handle) {
  lua::PushNewTable(state_);
  int original_registry_len = lua::Len(state_, LUA_REGISTRYINDEX);
  std::unique_ptr<lua::Handle> handle(new lua::Handle(state_));
  ASSERT_EQ(lua::Len(state_, LUA_REGISTRYINDEX), original_registry_len + 1);
  ASSERT_EQ(lua::GetTop(state_), 0);
  handle->Get();
  ASSERT_EQ(lua::GetTop(state_), 1);
  ASSERT_EQ(lua::GetType(state_, -1), lua::LuaType::Table);
  handle.reset();
  ASSERT_EQ(lua::Len(state_, LUA_REGISTRYINDEX), original_registry_len);
}
