// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua_yue/builtin_loader.h"
#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class YueGuiTest : public testing::Test {
 protected:
  void SetUp() override {
    luaL_openlibs(state_);
    yue::InsertBuiltinModuleLoader(state_);
  }

  lua::ManagedState state_;
};

TEST_F(YueGuiTest, CollectGarbageInCallback) {
  luaL_dostring(state_,
      "local gui = require('yue.gui')\n"
      "gui.MessageLoop.posttask(function()\n"
      "  collectgarbage()\n"
      "  gui.MessageLoop.posttask(function()\n"
      "    gui.MessageLoop.quit()\n"
      "  end)\n"
      "end)\n"
      "gui.MessageLoop.run()");
}
