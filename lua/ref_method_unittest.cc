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
           "add", RefMethod(&HostClass::Add, 1, true),
           "remove", RefMethod(&HostClass::Remove, 1, false));
  }
};

}  // namespace lua


TEST_F(RefMethodTest, AddRef) {
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
