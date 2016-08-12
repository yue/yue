// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <string>

#include "lua/callback.h"
#include "lua/wrappable.h"
#include "testing/gtest/include/gtest/gtest.h"

class WrappableTest : public testing::Test {
 protected:
  void SetUp() override {
    lua::SetTop(state_, 0);
  }

  lua::ManagedState state_;
};

class TestClass : public lua::Wrappable<TestClass> {
 public:
  static constexpr const char* name = "TestClass";
  static void SetMetaTable(lua::State* state, int index);

  int Method1(int n) {
    return n;
  }

  std::string Method2(const char* str) {
    return str;
  }

 private:
  friend class lua::Wrappable<TestClass>;

  TestClass() {}
  ~TestClass() override {}

  DISALLOW_COPY_AND_ASSIGN(TestClass);
};

void TestClass::SetMetaTable(lua::State* state, int index) {
  RawSet(state, index,
         "new", &TestClass::NewInstance<>,
         "method1", &TestClass::Method1,
         "method2", &TestClass::Method2);
}

TEST_F(WrappableTest, PushNewClass) {
  TestClass::PushNewClass(state_);
  EXPECT_EQ(lua::GetTop(state_), 1);
  EXPECT_EQ(lua::GetType(state_, -1), lua::LuaType::Table);
  lua::RawGet(state_, -1, "method1", "method2", "__gc", "__index");
  EXPECT_TRUE(lua::Compare(state_, 1, -1, lua::CompareOp::EQ));
  EXPECT_EQ(lua::GetType(state_, -1), lua::LuaType::Table);
  EXPECT_EQ(lua::GetType(state_, -2), lua::LuaType::Function);
  EXPECT_EQ(lua::GetType(state_, -3), lua::LuaType::Function);
  EXPECT_EQ(lua::GetType(state_, -4), lua::LuaType::Function);
}
