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
  static void BuildMetaTable(lua::State* state, int index);

  int Method1(int n) {
    return n;
  }

  std::string Method2(const char* str) {
    return str;
  }

  void SetPtr(int* ptr) {
    ptr_ = ptr;
  }

 private:
  friend class lua::Wrappable<TestClass>;

  explicit TestClass(lua::State* state)
      : lua::Wrappable<TestClass>(state), ptr_(nullptr) {}
  ~TestClass() override {
    if (ptr_)
      *ptr_ = 456;
  }

  int* ptr_;

  DISALLOW_COPY_AND_ASSIGN(TestClass);
};

void TestClass::BuildMetaTable(lua::State* state, int index) {
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

TEST_F(WrappableTest, PushNewInstanceInC) {
  TestClass::PushNewClass(state_);
  lua::PopAndIgnore(state_, 1);
  lua::Push(state_, TestClass::NewInstance(state_));
  EXPECT_EQ(lua::GetTop(state_), 1);
  EXPECT_EQ(lua::GetType(state_, 1), lua::LuaType::UserData);
  lua::GetMetaTable(state_, 1);
  EXPECT_EQ(lua::GetTop(state_), 2);
  EXPECT_EQ(lua::GetType(state_, 2), lua::LuaType::Table);

  ASSERT_TRUE(lua::PGet(state_, 1, "method1"));
  int result = 456;
  ASSERT_TRUE(lua::PCall(state_, &result, lua::ValueOnStack(state_, 1), 123));
  ASSERT_EQ(result, 123);
}

TEST_F(WrappableTest, PushNewInstanceInLua) {
  TestClass::PushNewClass(state_);
  ASSERT_TRUE(lua::PGet(state_, 1, "new"));
  TestClass* instance;
  EXPECT_TRUE(lua::PCall(state_, &instance));
  EXPECT_NE(instance, nullptr);

  lua::Push(state_, instance);
  ASSERT_TRUE(lua::PGet(state_, 1, "method2"));
  std::string result;
  ASSERT_TRUE(lua::PCall(state_, &result, instance, 123));
  ASSERT_EQ(result, "123");
}

TEST_F(WrappableTest, GC) {
  TestClass::PushNewClass(state_);
  TestClass* instance = TestClass::NewInstance(state_);

  int changed = 123;
  instance->SetPtr(&changed);

  lua::CollectGarbage(state_);
  ASSERT_EQ(changed, 456);
}
