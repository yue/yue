// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/lua.h"
#include "testing/gtest/include/gtest/gtest.h"

class RefMethodTest : public testing::Test {
 protected:
  void SetUp() override {
    lua::SetTop(state_, 0);
  }

  lua::ManagedState state_;
};

class ParamClass : public base::RefCounted<ParamClass> {
 public:
  explicit ParamClass(int* ptr) : ptr_(ptr) {}

 protected:
  friend class base::RefCounted<ParamClass>;

  virtual ~ParamClass() { *ptr_ = 88; }

 private:
  int* ptr_;
};

class HostClass : public base::RefCounted<HostClass> {
 public:
  HostClass() {}

  void Add(ParamClass*) {}
  void Remove(ParamClass*) {}
  void Set(ParamClass*) {}

 protected:
  friend class base::RefCounted<HostClass>;

  virtual ~HostClass() {}
};

namespace lua {

template<>
struct Type<ParamClass> {
  static constexpr const char* name = "ParamClass";
  static void BuildMetaTable(State* state, int index) {
  }
};

template<>
struct Type<HostClass> {
  static constexpr const char* name = "HostClass";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "new", &CreateOnHeap<HostClass>,
           "add", RefMethod(state, &HostClass::Add, RefType::Ref),
           "remove", RefMethod(state, &HostClass::Remove, RefType::Deref),
           "set", RefMethod(state, &HostClass::Set, RefType::Reset, "key"));
  }
};

}  // namespace lua

TEST_F(RefMethodTest, AddRemove) {
  int result = 0;
  ParamClass* float_ptr = new ParamClass(&result);
  lua::Push(state_, new HostClass);

  ASSERT_TRUE(lua::PGet(state_, 1, "add"));
  ASSERT_TRUE(lua::PCall(state_, nullptr,
                         lua::ValueOnStack(state_, 1),
                         float_ptr));

  lua::CollectGarbage(state_);
  ASSERT_EQ(result, 0);

  ASSERT_TRUE(lua::PGet(state_, 1, "remove"));
  ASSERT_TRUE(lua::PCall(state_, nullptr,
                         lua::ValueOnStack(state_, 1),
                         float_ptr));

  lua::CollectGarbage(state_);
  ASSERT_EQ(result, 88);
}

TEST_F(RefMethodTest, Set) {
  int result = 0;
  lua::Push(state_, new HostClass);

  ASSERT_TRUE(lua::PGet(state_, 1, "set"));
  ASSERT_TRUE(lua::PCall(state_, nullptr,
                         lua::ValueOnStack(state_, 1),
                         new ParamClass(&result)));

  lua::CollectGarbage(state_);
  ASSERT_EQ(result, 0);

  int new_result = 0;

  ASSERT_TRUE(lua::PGet(state_, 1, "set"));
  ASSERT_TRUE(lua::PCall(state_, nullptr,
                         lua::ValueOnStack(state_, 1),
                         new ParamClass(&new_result)));

  lua::CollectGarbage(state_);
  ASSERT_EQ(result, 88);
  ASSERT_EQ(new_result, 0);

  lua::SetTop(state_, 0);
  lua::CollectGarbage(state_);
  // On lua5.1 it takes a few cycles to collect the refs table.
  lua::CollectGarbage(state_);
  lua::CollectGarbage(state_);
  ASSERT_EQ(new_result, 88);
}
