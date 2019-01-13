// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class PickerTest : public testing::Test {
 protected:
  void SetUp() override {
    picker_ = new nu::Picker();
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::Picker> picker_;
};

TEST_F(PickerTest, Empty) {
  EXPECT_EQ(picker_->GetSelectedItemIndex(), -1);
}

TEST_F(PickerTest, SelectOneByDefault) {
  picker_->AddItem("item");
  EXPECT_EQ(picker_->GetSelectedItem(), "item");
  EXPECT_EQ(picker_->GetSelectedItemIndex(), 0);
}

TEST_F(PickerTest, AddRemove) {
  picker_->AddItem("item1");
  picker_->AddItem("item2");
  picker_->AddItem("item3");
  EXPECT_EQ(picker_->GetItems().size(), static_cast<size_t>(3));
  picker_->RemoveItemAt(1);
  EXPECT_EQ(picker_->GetItems().size(), static_cast<size_t>(2));
  picker_->RemoveItemAt(0);
  EXPECT_EQ(picker_->GetItems().size(), static_cast<size_t>(1));
}

TEST_F(PickerTest, NoDuplicateItem) {
  picker_->AddItem("item");
  picker_->AddItem("item");
  EXPECT_EQ(picker_->GetItems().size(), static_cast<size_t>(1));
}

TEST_F(PickerTest, SelectItemShouldNotEmitEvent) {
  picker_->AddItem("item1");
  picker_->AddItem("item2");
  bool emitted = false;
  picker_->on_selection_change.Connect([&emitted](nu::Picker*) {
    emitted = true;
  });
  picker_->SelectItemAt(1);
  EXPECT_FALSE(emitted);
}
