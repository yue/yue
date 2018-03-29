// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/lua.h"
#include "testing/gtest/include/gtest/gtest.h"

class IndexTest : public testing::Test {
 protected:
  void SetUp() override {
    lua::SetTop(state_, 0);
  }

  lua::ManagedState state_;
};

class PropertiesClass : public base::RefCounted<PropertiesClass> {
 public:
  PropertiesClass() {}

  int property = 0;
  std::function<void()> func;

 private:
  friend class base::RefCounted<PropertiesClass>;

  ~PropertiesClass() {}
};

namespace lua {

template<>
struct Type<PropertiesClass> {
  static constexpr const char* name = "PropertiesClass";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable, "new", &CreateOnHeap<PropertiesClass>);
    RawSetProperty(state, metatable,
                   "property", &PropertiesClass::property,
                   "func", &PropertiesClass::func);
  }
};

}  // namespace lua

TEST_F(IndexTest, Index) {
  lua::Push(state_, lua::MetaTable<PropertiesClass>());
  lua::RawGet(state_, 1, "__index");
  EXPECT_EQ(lua::GetType(state_, -1), lua::LuaType::Function);
  lua::RawGet(state_, 1, "__newindex");
  EXPECT_EQ(lua::GetType(state_, -1), lua::LuaType::Function);

  lua::Push(state_, new PropertiesClass);
  int property = -1;
  ASSERT_TRUE(lua::PGetAndPop(state_, -1, "property", &property));
  EXPECT_EQ(property, 0);
  ASSERT_TRUE(lua::PSet(state_, -1, "property", 123));
  ASSERT_TRUE(lua::PGetAndPop(state_, -1, "property", &property));
  EXPECT_EQ(property, 123);
}

TEST_F(IndexTest, DefaultIndex) {
  lua::Push(state_, new PropertiesClass);
  ASSERT_TRUE(lua::PGet(state_, -1, "new"));
  EXPECT_EQ(lua::GetType(state_, -1), lua::LuaType::Function);
}

TEST_F(IndexTest, NewIndexFailure) {
  lua::Push(state_, new PropertiesClass);
  ASSERT_FALSE(lua::PSet(state_, -1, "property", "string"));
  std::string error;
  ASSERT_TRUE(lua::Pop(state_, &error));
  ASSERT_EQ(error, "error converting string to integer");
}

TEST_F(IndexTest, IndexCache) {
  PropertiesClass* inst = new PropertiesClass;
  lua::Push(state_, inst);

  ASSERT_TRUE(lua::PGet(state_, 1, "func"));
  EXPECT_EQ(lua::GetType(state_, 2), lua::LuaType::Nil)
      << "Cached property should be nil before assignment";

  lua::Push(state_, std::function<void()>());
  ASSERT_TRUE(lua::PSet(state_, 1, "func", lua::ValueOnStack(state_, 3)));
  ASSERT_TRUE(lua::PGet(state_, 1, "func"));
  EXPECT_TRUE(lua::Compare(state_, 3, 4, lua::CompareOp::EQ))
      << "Cached property should return the original assigned one";
}

TEST_F(IndexTest, CustomData) {
  lua::Push(state_, new PropertiesClass);
  ASSERT_TRUE(lua::PSet(state_, 1, "custom", "data"));
  std::string data;
  ASSERT_TRUE(lua::PGetAndPop(state_, 1, "custom", &data));
  EXPECT_EQ(data, "data");
}

TEST_F(IndexTest, MemberCanBeGarbageCollected) {
  PropertiesClass* inst = new PropertiesClass;
  lua::Push(state_, inst);
  std::function<int(int)> func = [](int i) { return i; };
  lua::Push(state_, func);

  ASSERT_TRUE(lua::PSet(state_, 1, "func", lua::ValueOnStack(state_, 2)));
  ASSERT_TRUE(lua::PGet(state_, 1, "func"));
  int ret = -1;
  ASSERT_TRUE(lua::PCall(state_, &ret, 123));
  EXPECT_EQ(ret, 123);

  lua::SetTop(state_, 1);
  lua::CollectGarbage(state_);
  lua::CollectGarbage(state_);

  ASSERT_TRUE(lua::PGet(state_, 1, "func"));
  ret = -1;
  ASSERT_TRUE(lua::PCall(state_, &ret, 123));
  EXPECT_EQ(ret, 123);
}
