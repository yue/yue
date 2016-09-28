// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"
#include "yue/builtin_loader.h"

class YueMessageLoopTest : public testing::Test {
 protected:
  void SetUp() override {
    luaL_openlibs(state_);
    yue::InsertBuiltinModuleLoader(state_);
  }

  lua::ManagedState state_;
};

TEST_F(YueMessageLoopTest, Require) {
  ASSERT_FALSE(luaL_dostring(state_, "require('yue.messageloop')"));
}

TEST_F(YueMessageLoopTest, New) {
  ASSERT_FALSE(luaL_dostring(state_, "require('yue.messageloop').new('ui')"));
  ASSERT_TRUE(luaL_dostring(state_, "require('yue.messageloop').new('none')"));
  std::string error;
  ASSERT_TRUE(lua::Pop(state_, &error));
  EXPECT_EQ(error, "MessageLoop of type 'none' is not supported");
  ASSERT_TRUE(luaL_dostring(state_, "require('yue.messageloop').new('ui')"));
  ASSERT_TRUE(lua::Pop(state_, &error));
  EXPECT_EQ(error, "MessageLoop is alraedy created");
}

TEST_F(YueMessageLoopTest, Run) {
  ASSERT_FALSE(luaL_dostring(state_,
        "local loop = require('yue.messageloop').new('ui')\n"
        "local flag = false\n"
        "loop:post(function()\n"
          "flag = true\n"
          "loop:quit()\n"
        "end)\n"
        "loop:run()\n"
        "return flag"));
  bool flag;
  ASSERT_TRUE(lua::Pop(state_, &flag));
  EXPECT_EQ(flag, true);
}
