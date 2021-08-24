// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <memory>

#include "lua/lua.h"
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
  lua::NewTable(state_);
  auto handle = std::make_unique<lua::Persistent>(state_, -1);
  ASSERT_GT(lua::RawLen(state_, LUA_REGISTRYINDEX), original_registry_len);

  lua::PopAndIgnore(state_, 1);
  ASSERT_EQ(lua::GetTop(state_), 0);
  lua::CollectGarbage(state_);
  lua::CollectGarbage(state_);

  handle->Push();
  ASSERT_EQ(lua::GetTop(state_), 1);
  ASSERT_EQ(lua::GetType(state_, -1), lua::LuaType::Table);
}

TEST_F(HandleTest, GC) {
  int changed = 123;
  lua_newuserdata(state_, 8);
  lua::NewTable(state_);
  std::function<void()> on_gc = [&changed] { changed = 456; };
  lua::RawSet(state_, 2, "__gc", on_gc);
  lua::SetMetaTable(state_, 1);
  ASSERT_EQ(lua::GetTop(state_), 1);
  lua::SetTop(state_, 0);

  lua::CollectGarbage(state_);
  ASSERT_EQ(changed, 456);
}

TEST_F(HandleTest, Weak) {
  int changed = 123;
  lua_newuserdata(state_, 8);
  lua::NewTable(state_);
  std::function<void()> on_gc = [&changed] { changed = 456; };
  lua::RawSet(state_, 2, "__gc", on_gc);
  lua::SetMetaTable(state_, 1);
  ASSERT_EQ(lua::GetTop(state_), 1);

  lua::Weak ref(state_, -1);
  ASSERT_EQ(lua::GetTop(state_), 1);

  ref.Push();
  EXPECT_EQ(lua::GetTop(state_), 2);
  EXPECT_TRUE(lua::Compare(state_, 1, 2, lua::CompareOp::EQ));
  lua::SetTop(state_, 0);

  lua::CollectGarbage(state_);
  ASSERT_EQ(changed, 456);
}
