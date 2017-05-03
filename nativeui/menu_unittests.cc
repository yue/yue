// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class MenuTest : public testing::Test {
 protected:
  void SetUp() override {
    menu_ = new nu::Menu;
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::Menu> menu_;
};

TEST_F(MenuTest, ItemCount) {
  EXPECT_EQ(menu_->ItemCount(), 0);
  menu_->Append(new nu::MenuItem(nu::MenuItem::Type::Label));
  EXPECT_EQ(menu_->ItemCount(), 1);
  menu_->Remove(menu_->ItemAt(1));
  EXPECT_EQ(menu_->ItemCount(), 1);
  menu_->Remove(menu_->ItemAt(0));
  EXPECT_EQ(menu_->ItemCount(), 0);
}
