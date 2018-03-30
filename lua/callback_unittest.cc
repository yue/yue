// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/lua.h"
#include "testing/gtest/include/gtest/gtest.h"

class CallbackTest : public testing::Test {
 protected:
  void SetUp() override {
    lua::SetTop(state_, 0);
  }

  lua::ManagedState state_;
};

TEST_F(CallbackTest, PCallWithInvalidValue) {
  std::string str;
  lua::Push(state_, nullptr);
  EXPECT_FALSE(lua::PCall(state_, nullptr));
  ASSERT_EQ(lua::GetTop(state_), 1);
  ASSERT_TRUE(lua::Pop(state_, &str));
  ASSERT_EQ(str, "attempt to call a nil value");
}

void FunctionWithArgs(int, const std::string&) {
}

TEST_F(CallbackTest, PCallWithInsufficientArgs) {
  std::string str;
  lua::Push(state_, &FunctionWithArgs);
  EXPECT_FALSE(lua::PCall(state_, nullptr, 123));
  ASSERT_TRUE(lua::Pop(state_, &str));
  ASSERT_EQ(str, "insufficient args, only 1 supplied");
  ASSERT_EQ(lua::GetTop(state_), 0);

  lua::Push(state_, &FunctionWithArgs);
  EXPECT_FALSE(lua::PCall(state_, nullptr));
  ASSERT_TRUE(lua::Pop(state_, &str));
  ASSERT_EQ(str, "insufficient args, only 0 supplied");
  ASSERT_EQ(lua::GetTop(state_), 0);
}

TEST_F(CallbackTest, PCallWithWrongArgs) {
  std::string str;
  lua::Push(state_, &FunctionWithArgs);
  EXPECT_FALSE(lua::PCall(state_, nullptr, "test", 123));
  ASSERT_TRUE(lua::Pop(state_, &str));
  ASSERT_EQ(str, "error converting arg at index 1 from string to integer");
  ASSERT_EQ(lua::GetTop(state_), 0);
}

void FunctionWithState(lua::State* state, int, const std::string&) {
}

TEST_F(CallbackTest, PCallWithState) {
  std::string str;
  lua::Push(state_, &FunctionWithState);
  EXPECT_FALSE(lua::PCall(state_, nullptr, 123));
  ASSERT_TRUE(lua::Pop(state_, &str));
  ASSERT_EQ(str, "insufficient args, only 1 supplied");
  ASSERT_EQ(lua::GetTop(state_), 0);

  lua::Push(state_, &FunctionWithState);
  EXPECT_FALSE(lua::PCall(state_, nullptr));
  ASSERT_TRUE(lua::Pop(state_, &str));
  ASSERT_EQ(str, "insufficient args, only 0 supplied");
  ASSERT_EQ(lua::GetTop(state_), 0);

  lua::Push(state_, &FunctionWithState);
  EXPECT_FALSE(lua::PCall(state_, nullptr, "test", 123));
  ASSERT_TRUE(lua::Pop(state_, &str));
  ASSERT_EQ(str, "error converting arg at index 1 from string to integer");
  ASSERT_EQ(lua::GetTop(state_), 0);
}

void FunctionThrowError(lua::CallContext* context) {
  lua::Push(context->state, "custom error");
  context->has_error = true;
}

TEST_F(CallbackTest, PCallWithCustomError) {
  std::string str;
  lua::Push(state_, &FunctionThrowError);
  EXPECT_FALSE(lua::PCall(state_, nullptr));
  ASSERT_TRUE(lua::Pop(state_, &str));
  ASSERT_EQ(str, "custom error");
  ASSERT_EQ(lua::GetTop(state_), 0);
}

void FunctionWithoutReturnValue() {
}

TEST_F(CallbackTest, PCallWithoutReturnValue) {
  lua::Push(state_, &FunctionWithoutReturnValue);
  EXPECT_TRUE(lua::PCall(state_, nullptr));
  ASSERT_EQ(lua::GetTop(state_), 0);
  lua::Push(state_, &FunctionWithArgs);
  EXPECT_TRUE(lua::PCall(state_, nullptr, 123, "test"));
  ASSERT_EQ(lua::GetTop(state_), 0);
}

int FunctionReturnsInt(int num) {
  return num;
}

std::string FunctionReturnsString(base::StringPiece str) {
  return std::string(str.data(), str.size());
}

TEST_F(CallbackTest, PCallWithReturnValue) {
  int num = 42;
  int out = 0;
  lua::Push(state_, &FunctionReturnsInt);
  ASSERT_TRUE(lua::PCall(state_, &out, num));
  EXPECT_EQ(num, out);
  ASSERT_EQ(lua::GetTop(state_), 0);

  base::StringPiece str = "valar morghulis";
  std::string str_out;
  lua::Push(state_, &FunctionReturnsString);
  ASSERT_TRUE(lua::PCall(state_, &str_out, str));
  EXPECT_EQ(str_out, str);
  ASSERT_EQ(lua::GetTop(state_), 0);
}

TEST_F(CallbackTest, PCallFailToConvertReturnValue) {
  int num = 42;
  bool out;
  lua::Push(state_, &FunctionReturnsInt);
  ASSERT_FALSE(lua::PCall(state_, &out, num));
  EXPECT_EQ(lua::GetTop(state_), 1);
  std::string error;
  ASSERT_TRUE(lua::Pop(state_, &error));
  EXPECT_EQ(error, "error converting return value from number to boolean");
}

std::tuple<std::string, int> FunctionReturnsTuple(
    const std::string& str, int number) {
  return std::make_tuple(str, number);
}

TEST_F(CallbackTest, PCallWithMultipleReturnValues) {
  lua::Push(state_, &FunctionReturnsTuple);
  std::tuple<std::string, int> out;
  ASSERT_TRUE(lua::PCall(state_, &out, "str", 123));
  EXPECT_EQ(std::get<0>(out), "str");
  EXPECT_EQ(std::get<1>(out), 123);
  ASSERT_EQ(lua::GetTop(state_), 0);
}

TEST_F(CallbackTest, Callback) {
  lua::Push(state_, &FunctionReturnsInt);
  std::function<int(int)> callback;
  ASSERT_TRUE(lua::Pop(state_, &callback));
  ASSERT_EQ(lua::GetTop(state_), 0);
  ASSERT_EQ(callback(123), 123);
  ASSERT_EQ(lua::GetTop(state_), 0);
  ASSERT_EQ(callback(456), 456);
  ASSERT_EQ(lua::GetTop(state_), 0);
}

TEST_F(CallbackTest, CallbackWithVoidReturn) {
  int ret = 123;
  std::function<void()> func = [&ret] { ret = 456; };
  lua::Push(state_, func);
  std::function<void(int)> callback;
  ASSERT_TRUE(lua::Pop(state_, &callback));
  ASSERT_EQ(lua::GetTop(state_), 0);
  callback(123);
  ASSERT_EQ(ret, 456);
  ret = 123;
  callback(123);
  ASSERT_EQ(ret, 456);
  ASSERT_EQ(lua::GetTop(state_), 0);
}

std::tuple<int, int, int, int> FunctionReturnsMultipleIntegers(
    int i1, int i2, int i3, int i4) {
  return std::make_tuple(i1, i2, i3, i4);
}

TEST_F(CallbackTest, CallbackWithMultipleReturns) {
  lua::Push(state_, &FunctionReturnsMultipleIntegers);
  std::function<std::tuple<int, int>(int, int, int, int)> callback1;
  ASSERT_TRUE(lua::To(state_, 1, &callback1));
  ASSERT_EQ(lua::GetTop(state_), 1);
  std::tuple<int, int> ret1 = callback1(1, 2, 3, 4);
  EXPECT_EQ(lua::GetTop(state_), 1);
  EXPECT_EQ(std::get<0>(ret1), 1);
  EXPECT_EQ(std::get<1>(ret1), 2);
  std::function<int(int, int, int, int)> callback2;
  ASSERT_TRUE(lua::To(state_, 1, &callback2));
  ASSERT_EQ(callback2(2, 3, 4, 5), 2);
  EXPECT_EQ(lua::GetTop(state_), 1);
  std::function<void(void)> callback3;
  ASSERT_TRUE(lua::To(state_, 1, &callback3));
  callback3();
  EXPECT_EQ(lua::GetTop(state_), 1);
}

TEST_F(CallbackTest, CallbackIsStoredStrong) {
  lua::Push(state_, &FunctionReturnsInt);
  std::function<int(int)> callback;
  ASSERT_TRUE(lua::To(state_, 1, &callback));
  EXPECT_EQ(callback(123), 123);
  lua::SetTop(state_, 0);
  lua::CollectGarbage(state_);
  EXPECT_EQ(callback(123), 123);
}

TEST_F(CallbackTest, WeakFunctionIsStoredWeak) {
  lua::Push(state_, &FunctionReturnsInt);
  std::function<int(int)> callback;
  ASSERT_TRUE(lua::ToWeakFunction(state_, 1, &callback));
  EXPECT_EQ(callback(123), 123);
  lua::SetTop(state_, 0);
  lua::CollectGarbage(state_);
  EXPECT_EQ(callback(123), 0);
}
