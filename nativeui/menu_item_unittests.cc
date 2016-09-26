// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "base/bind.h"
#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class MenuItemTest : public testing::Test {
 protected:
  nu::State state_;
};

void OnClick(bool* ptr) {
  *ptr = true;
}

TEST_F(MenuItemTest, Click) {
  scoped_refptr<nu::MenuItem> item = new nu::MenuItem(nu::MenuItem::Label);
  bool clicked = false;
  item->on_click.Connect(base::Bind(&OnClick, &clicked));
  item->Click();
  EXPECT_TRUE(clicked);
}

TEST_F(MenuItemTest, Label) {
  scoped_refptr<nu::MenuItem> item = new nu::MenuItem(nu::MenuItem::Label);
  item->SetLabel("test");
  EXPECT_EQ(item->GetLabel(), "test");
}

TEST_F(MenuItemTest, Checkbox) {
  scoped_refptr<nu::MenuItem> item = new nu::MenuItem(nu::MenuItem::CheckBox);
  EXPECT_FALSE(item->IsChecked());
  item->SetChecked(true);
  EXPECT_TRUE(item->IsChecked());
  item->Click();
  EXPECT_FALSE(item->IsChecked());
}

TEST_F(MenuItemTest, Visible) {
  scoped_refptr<nu::MenuItem> item = new nu::MenuItem(nu::MenuItem::Label);
  EXPECT_TRUE(item->IsVisible());
  item->SetVisible(false);
  EXPECT_FALSE(item->IsVisible());
}

TEST_F(MenuItemTest, Radio) {
  scoped_refptr<nu::MenuItem> item = new nu::MenuItem(nu::MenuItem::Radio);
  item->Click();
  EXPECT_TRUE(item->IsChecked());
}

TEST_F(MenuItemTest, RadioGroup) {
  scoped_refptr<nu::Menu> menu = new nu::Menu;
  menu->Append(new nu::MenuItem(nu::MenuItem::Radio));
  menu->Append(new nu::MenuItem(nu::MenuItem::Separator));
  menu->Append(new nu::MenuItem(nu::MenuItem::Radio));
  menu->Append(new nu::MenuItem(nu::MenuItem::Radio));
  menu->item_at(0)->Click();
  menu->item_at(2)->Click();
  EXPECT_TRUE(menu->item_at(0)->IsChecked());
  EXPECT_TRUE(menu->item_at(2)->IsChecked());
  EXPECT_FALSE(menu->item_at(3)->IsChecked());
  menu->item_at(3)->Click();
  EXPECT_TRUE(menu->item_at(0)->IsChecked());
  EXPECT_FALSE(menu->item_at(2)->IsChecked());
  EXPECT_TRUE(menu->item_at(3)->IsChecked());
}
