// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class TabTest : public testing::Test {
 protected:
  void SetUp() override {
    tab_ = new nu::Tab();
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::Tab> tab_;
};

TEST_F(TabTest, Empty) {
  EXPECT_EQ(tab_->PageCount(), 0);
  EXPECT_EQ(tab_->GetSelectedPageIndex(), -1);
  EXPECT_EQ(tab_->GetSelectedPage(), nullptr);
}

TEST_F(TabTest, AddAndSelect) {
  scoped_refptr<nu::View> page = new nu::Container;
  tab_->AddPage("Tab", page.get());
  EXPECT_EQ(tab_->PageCount(), 1);
  EXPECT_EQ(tab_->GetSelectedPageIndex(), 0);
  EXPECT_EQ(tab_->GetSelectedPage(), page.get());
  tab_->AddPage("Tab 2", new nu::Container);
  EXPECT_EQ(tab_->PageCount(), 2);
  EXPECT_EQ(tab_->GetSelectedPageIndex(), 0);
}

TEST_F(TabTest, RemoveAndSelect) {
  tab_->AddPage("Tab 1", new nu::Container);
  tab_->AddPage("Tab 2", new nu::Container);
  tab_->AddPage("Tab 3", new nu::Container);
  tab_->RemovePage(tab_->PageAt(2));
  EXPECT_EQ(tab_->PageCount(), 2);
  EXPECT_EQ(tab_->GetSelectedPageIndex(), 0);
}

TEST_F(TabTest, SelectPage) {
  tab_->AddPage("Tab 1", new nu::Container);
  tab_->AddPage("Tab 2", new nu::Container);
  tab_->AddPage("Tab 3", new nu::Container);
  tab_->SelectPageAt(1);
  EXPECT_EQ(tab_->GetSelectedPageIndex(), 1);
  tab_->SelectPageAt(10);
  EXPECT_EQ(tab_->GetSelectedPageIndex(), 1);
}

TEST_F(TabTest, SelectPageShouldEmitEvent) {
  tab_->AddPage("Tab 1", new nu::Container);
  tab_->AddPage("Tab 2", new nu::Container);
  bool emitted = false;
  tab_->on_selected_page_change.Connect([&emitted](nu::Tab*) {
    emitted = true;
  });
  tab_->SelectPageAt(1);
  EXPECT_TRUE(emitted);
}
