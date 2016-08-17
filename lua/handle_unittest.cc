// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <memory>

#include "base/bind.h"
#include "lua/handle.h"
#include "lua/callback.h"
#include "lua/table.h"
#include "testing/gtest/include/gtest/gtest.h"

class HandleTest : public testing::Test {
 protected:
  void SetUp() override {
    lua::SetTop(state_, 0);
  }

  lua::ManagedState state_;
};

TEST_F(HandleTest, Persistent) {
  size_t original_registry_len = lua::RawLen(state_, LUA_REGISTRYINDEX);
  lua::PushNewTable(state_);
  std::unique_ptr<lua::Persistent> handle(lua::Persistent::New(state_, -1));
  ASSERT_EQ(lua::RawLen(state_, LUA_REGISTRYINDEX), original_registry_len + 1);
  lua::PopAndIgnore(state_, 1);
  ASSERT_EQ(lua::GetTop(state_), 0);
  lua::CollectGarbage(state_);
  lua::CollectGarbage(state_);
  handle->Push(state_);
  ASSERT_EQ(lua::GetTop(state_), 1);
  ASSERT_EQ(lua::GetType(state_, -1), lua::LuaType::Table);
  handle.reset();
  ASSERT_EQ(lua::RawLen(state_, LUA_REGISTRYINDEX), original_registry_len);
}

void OnGC(int* ptr) {
  if (ptr)
    *ptr = 456;
}

TEST_F(HandleTest, GC) {
  int changed = 123;
  lua::PushNewTable(state_);
  lua::PushNewTable(state_);
  lua::RawSet(state_, 2, "__gc", base::Bind(&OnGC, &changed));
  lua::SetMetaTable(state_, 1);
  ASSERT_EQ(lua::GetTop(state_), 1);
  lua::SetTop(state_, 0);

  lua::CollectGarbage(state_);
  ASSERT_EQ(changed, 456);
}

TEST_F(HandleTest, Weak) {
  int changed = 123;
  lua::PushNewTable(state_);
  lua::PushNewTable(state_);
  lua::RawSet(state_, 2, "__gc", base::Bind(&OnGC, &changed));
  lua::SetMetaTable(state_, 1);
  ASSERT_EQ(lua::GetTop(state_), 1);
  lua::Weak handle(state_, -1);
  ASSERT_EQ(lua::GetTop(state_), 1);

  handle.Push(state_);
  EXPECT_EQ(lua::GetTop(state_), 2);
  EXPECT_TRUE(lua::Compare(state_, 1, 2, lua::CompareOp::EQ));
  lua::SetTop(state_, 0);

  lua::CollectGarbage(state_);
  ASSERT_EQ(changed, 456);
}
