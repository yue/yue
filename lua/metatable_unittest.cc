// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <string>

#include "lua/callback.h"
#include "lua/metatable.h"
#include "testing/gtest/include/gtest/gtest.h"

class MetaTableTest : public testing::Test {
 protected:
  void SetUp() override {
    lua::SetTop(state_, 0);
  }

  lua::ManagedState state_;
};

class TestClass : public base::RefCounted<TestClass> {
 public:
  TestClass() : ptr_(nullptr) {}

  int Method1(int n) {
    return n;
  }

  std::string Method2(const char* str) {
    return str;
  }

  void SetPtr(int* ptr) {
    ptr_ = ptr;
  }

 protected:
  friend class base::RefCounted<TestClass>;
  virtual ~TestClass() {
    if (ptr_)
      *ptr_ = 456;
  }

 private:
  int* ptr_;

  DISALLOW_COPY_AND_ASSIGN(TestClass);
};

namespace lua {

template<>
struct Type<TestClass> {
  static constexpr const char* name = "TestClass";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "new", &MetaTable<TestClass>::NewInstance<>,
           "method1", &TestClass::Method1,
           "method2", &TestClass::Method2);
  }
};

}  // namespace lua

TEST_F(MetaTableTest, PushNewClass) {
  lua::MetaTable<TestClass>::Push(state_);
  EXPECT_EQ(lua::GetTop(state_), 1);
  EXPECT_EQ(lua::GetType(state_, -1), lua::LuaType::Table);
  lua::RawGet(state_, -1, "method1", "method2", "__gc", "__index");
  EXPECT_TRUE(lua::Compare(state_, 1, -1, lua::CompareOp::EQ));
  EXPECT_EQ(lua::GetType(state_, -1), lua::LuaType::Table);
  EXPECT_EQ(lua::GetType(state_, -2), lua::LuaType::Function);
  EXPECT_EQ(lua::GetType(state_, -3), lua::LuaType::Function);
  EXPECT_EQ(lua::GetType(state_, -4), lua::LuaType::Function);
}

TEST_F(MetaTableTest, PushNewInstanceInC) {
  lua::MetaTable<TestClass>::Push(state_);
  lua::PopAndIgnore(state_, 1);
  lua::Push(state_, lua::MetaTable<TestClass>::NewInstance(state_));
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

TEST_F(MetaTableTest, PushNewInstanceInLua) {
  lua::MetaTable<TestClass>::Push(state_);
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

TEST_F(MetaTableTest, GC) {
  lua::MetaTable<TestClass>::Push(state_);
  TestClass* instance = lua::MetaTable<TestClass>::NewInstance(state_);

  int changed = 123;
  instance->SetPtr(&changed);

  lua::CollectGarbage(state_);
  ASSERT_EQ(changed, 456);
}

TEST_F(MetaTableTest, GCWithCReference) {
  lua::MetaTable<TestClass>::Push(state_);
  scoped_refptr<TestClass> instance =
      lua::MetaTable<TestClass>::NewInstance(state_);

  int changed = 123;
  instance->SetPtr(&changed);

  lua::CollectGarbage(state_);
  ASSERT_EQ(changed, 123);
  instance = nullptr;
  ASSERT_EQ(changed, 456);
}

class DerivedClass : public TestClass {
 public:
  DerivedClass(int a, const char* b) : a_(a), b_(b) {}

  int A() const { return a_; }
  std::string B() const { return b_; }

 protected:
  ~DerivedClass() override {}

 private:
  int a_;
  std::string b_;
};

namespace lua {

template<>
struct Type<DerivedClass> {
  static constexpr const char* name = "DerivedClass";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "new", &MetaTable<DerivedClass>::NewInstance<int, const char*>,
           "a", &DerivedClass::A,
           "b", &DerivedClass::B);
  }
};

}  // namespace lua

TEST_F(MetaTableTest, IndependentDerivedClass) {
  lua::MetaTable<DerivedClass>::Push(state_);
  lua::PopAndIgnore(state_, 1);
  lua::Push(state_, lua::MetaTable<DerivedClass>::NewInstance(state_, 1, "b"));
  EXPECT_EQ(lua::GetType(state_, 1), lua::LuaType::UserData);

  ASSERT_TRUE(lua::PGet(state_, 1, "a"));
  int a = 0;
  ASSERT_TRUE(lua::PCall(state_, &a, lua::ValueOnStack(state_, 1)));
  ASSERT_EQ(a, 1);

  ASSERT_TRUE(lua::PGet(state_, 1, "b"));
  std::string b;
  ASSERT_TRUE(lua::PCall(state_, &b, lua::ValueOnStack(state_, 1)));
  ASSERT_EQ(b, "b");
}

TEST_F(MetaTableTest, DerivedClassInheritanceChain) {
  lua::MetaTable<DerivedClass>::Push<TestClass>(state_);
  EXPECT_EQ(lua::GetTop(state_), 1);
  std::string name;
  ASSERT_TRUE(lua::RawGetAndPop(state_, 1, "__name", &name));
  ASSERT_EQ(name, "DerivedClass");
  ASSERT_TRUE(lua::GetMetaTable(state_, -1));
  EXPECT_EQ(lua::GetTop(state_), 2);
  EXPECT_EQ(lua::GetType(state_, 2), lua::LuaType::Table);
  lua::RawGet(state_, -1, "__index");
  EXPECT_EQ(lua::GetTop(state_), 3);
  EXPECT_EQ(lua::GetType(state_, 3), lua::LuaType::Table);
  ASSERT_TRUE(lua::RawGetAndPop(state_, 3, "__name", &name));
  ASSERT_EQ(name, "TestClass");
}

TEST_F(MetaTableTest, DerivedClassMethods) {
  lua::MetaTable<DerivedClass>::Push<TestClass>(state_);
  DerivedClass* instance =
      lua::MetaTable<DerivedClass>::NewInstance(state_, 456, "str");
  lua::Push(state_, instance);
  EXPECT_EQ(lua::GetTop(state_), 2);
  ASSERT_TRUE(lua::PGet(state_, 2, "a", "b", "method1", "method2"));
  EXPECT_EQ(lua::GetType(state_, -1), lua::LuaType::Function);
  EXPECT_EQ(lua::GetType(state_, -2), lua::LuaType::Function);
  EXPECT_EQ(lua::GetType(state_, -3), lua::LuaType::Function);
  EXPECT_EQ(lua::GetType(state_, -4), lua::LuaType::Function);

  int result = 456;
  ASSERT_TRUE(lua::PGet(state_, 2, "method1"));
  ASSERT_TRUE(lua::PCall(state_, &result, lua::ValueOnStack(state_, 2), 123));
  EXPECT_EQ(result, 123);
  ASSERT_TRUE(lua::PGet(state_, 2, "a"));
  ASSERT_TRUE(lua::PCall(state_, &result, lua::ValueOnStack(state_, 2)));
  EXPECT_EQ(result, 456);
}

class DerivedClass2 : public DerivedClass {
 public:
  DerivedClass2() : DerivedClass(123, "test"), ptr_(nullptr) {}

  int C() const { return 789; }

  void SetPtr(int* ptr) {
    ptr_ = ptr;
  }

 private:
  ~DerivedClass2() override {
    if (ptr_)
      *ptr_ = 789;
  }

  int* ptr_;
};

namespace lua {

template<>
struct Type<DerivedClass2> {
  static constexpr const char* name = "DerivedClass2";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index, "new", &MetaTable<DerivedClass2>::NewInstance<>);
  }
};

}  // namespace lua

TEST_F(MetaTableTest, DeeplyDerivedClassInheritanceChain) {
  lua::MetaTable<DerivedClass2>::Push<DerivedClass, TestClass>(state_);
  EXPECT_EQ(lua::GetTop(state_), 1);
  std::string name;
  ASSERT_TRUE(lua::RawGetAndPop(state_, 1, "__name", &name));
  ASSERT_EQ(name, "DerivedClass2");
  ASSERT_TRUE(lua::GetMetaTable(state_, -1));
  EXPECT_EQ(lua::GetTop(state_), 2);
  EXPECT_EQ(lua::GetType(state_, 2), lua::LuaType::Table);
  lua::RawGet(state_, -1, "__index");
  EXPECT_EQ(lua::GetTop(state_), 3);
  EXPECT_EQ(lua::GetType(state_, 3), lua::LuaType::Table);
  ASSERT_TRUE(lua::RawGetAndPop(state_, 3, "__name", &name));
  ASSERT_EQ(name, "DerivedClass");
  ASSERT_TRUE(lua::GetMetaTable(state_, -1));
  EXPECT_EQ(lua::GetTop(state_), 4);
  EXPECT_EQ(lua::GetType(state_, 4), lua::LuaType::Table);
  lua::RawGet(state_, -1, "__index");
  EXPECT_EQ(lua::GetTop(state_), 5);
  EXPECT_EQ(lua::GetType(state_, 5), lua::LuaType::Table);
  ASSERT_TRUE(lua::RawGetAndPop(state_, 5, "__name", &name));
  ASSERT_EQ(name, "TestClass");
}

TEST_F(MetaTableTest, DeeplyDerivedClassGC) {
  lua::MetaTable<DerivedClass2>::Push<DerivedClass, TestClass>(state_);
  DerivedClass2* d2 = lua::MetaTable<DerivedClass2>::NewInstance(state_);
  int changed_d2 = 0;
  d2->SetPtr(&changed_d2);

  TestClass* b;
  lua::Push(state_, d2);
  ASSERT_TRUE(lua::To(state_, -1, &b));
  int changed_b = 0;
  b->SetPtr(&changed_b);

  lua::SetTop(state_, 0);
  lua::CollectGarbage(state_);
  EXPECT_EQ(changed_b, 456);
  EXPECT_EQ(changed_d2, 789);
}

TEST_F(MetaTableTest, BaseCallDerivedClassMethods) {
  lua::MetaTable<DerivedClass2>::Push<DerivedClass, TestClass>(state_);
  TestClass* b = lua::MetaTable<TestClass>::NewInstance(state_);
  ASSERT_TRUE(lua::PGet(state_, 1, "a"));
  ASSERT_FALSE(lua::PCall(state_, nullptr, b, 123));
  std::string error;
  ASSERT_TRUE(lua::Pop(state_, &error));
  ASSERT_EQ(error, "error converting arg at index 1 "
                   "from userdata to DerivedClass");
}

TEST_F(MetaTableTest, BaseConvertToDerivedClass) {
  lua::MetaTable<DerivedClass2>::Push<DerivedClass, TestClass>(state_);
  lua::Push(state_, lua::MetaTable<TestClass>::NewInstance(state_));
  DerivedClass2* d2;
  ASSERT_FALSE(lua::Pop(state_, &d2));
}

TEST_F(MetaTableTest, PushWithoutNewInstance) {
  lua::MetaTable<DerivedClass2>::Push<DerivedClass, TestClass>(state_);
  DerivedClass2* d2 = new DerivedClass2;
  lua::Push(state_, d2);
  TestClass* b;
  ASSERT_TRUE(lua::Pop(state_, &b));
  EXPECT_EQ(d2, b);
}

TEST_F(MetaTableTest, PushWithoutNewInstanceAfterGC) {
  lua::MetaTable<DerivedClass2>::Push<DerivedClass, TestClass>(state_);
  lua::Push(state_, new DerivedClass2);

  TestClass* b;
  ASSERT_TRUE(lua::To(state_, -1, &b));
  int changed = 0;
  b->SetPtr(&changed);

  scoped_refptr<TestClass> keep(b);
  lua::SetTop(state_, 0);
  lua::CollectGarbage(state_);
  EXPECT_EQ(changed, 0);

  lua::Push(state_, keep.get());
  keep = nullptr;
  ASSERT_TRUE(lua::To(state_, -1, &b));
  EXPECT_NE(b, nullptr);

  lua::SetTop(state_, 0);
  lua::CollectGarbage(state_);
  EXPECT_EQ(changed, 456);
}
