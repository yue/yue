// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class MenuItemTest : public testing::Test {
 protected:
  nu::Lifetime lifetime_;
  nu::State state_;
};

TEST_F(MenuItemTest, Click) {
  scoped_refptr<nu::MenuItem> item =
      new nu::MenuItem(nu::MenuItem::Type::Label);
  bool clicked = false;
  item->on_click.Connect([&clicked](nu::MenuItem*) {
    clicked = true;
  });
  item->Click();
  EXPECT_TRUE(clicked);
}

TEST_F(MenuItemTest, Label) {
  scoped_refptr<nu::MenuItem> item =
      new nu::MenuItem(nu::MenuItem::Type::Label);
  item->SetLabel("test");
  EXPECT_EQ(item->GetLabel(), "test");
}

TEST_F(MenuItemTest, Checkbox) {
  scoped_refptr<nu::MenuItem> item =
      new nu::MenuItem(nu::MenuItem::Type::Checkbox);
  EXPECT_FALSE(item->IsChecked());
  item->SetChecked(true);
  EXPECT_TRUE(item->IsChecked());
  item->Click();
  EXPECT_FALSE(item->IsChecked());
}

TEST_F(MenuItemTest, Visible) {
  scoped_refptr<nu::MenuItem> item =
      new nu::MenuItem(nu::MenuItem::Type::Label);
  EXPECT_TRUE(item->IsVisible());
  item->SetVisible(false);
  EXPECT_FALSE(item->IsVisible());
}

TEST_F(MenuItemTest, Radio) {
  scoped_refptr<nu::MenuItem> item =
      new nu::MenuItem(nu::MenuItem::Type::Radio);
  item->Click();
  EXPECT_TRUE(item->IsChecked());
}

TEST_F(MenuItemTest, RadioGroup) {
  scoped_refptr<nu::Menu> menu = new nu::Menu;
  menu->Append(new nu::MenuItem(nu::MenuItem::Type::Radio));
  menu->Append(new nu::MenuItem(nu::MenuItem::Type::Separator));
  menu->Append(new nu::MenuItem(nu::MenuItem::Type::Radio));
  menu->Append(new nu::MenuItem(nu::MenuItem::Type::Radio));
  menu->ItemAt(0)->Click();
  menu->ItemAt(2)->Click();
  EXPECT_TRUE(menu->ItemAt(0)->IsChecked());
  EXPECT_TRUE(menu->ItemAt(2)->IsChecked());
  EXPECT_FALSE(menu->ItemAt(3)->IsChecked());
  menu->ItemAt(3)->Click();
  EXPECT_TRUE(menu->ItemAt(0)->IsChecked());
  EXPECT_FALSE(menu->ItemAt(2)->IsChecked());
  EXPECT_TRUE(menu->ItemAt(3)->IsChecked());
}
