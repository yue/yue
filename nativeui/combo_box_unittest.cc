// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class ComboBoxTest : public testing::Test {
 protected:
  void SetUp() override {
    combobox_ = new nu::ComboBox();
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::ComboBox> combobox_;
};

TEST_F(ComboBoxTest, WorkLikePicker) {
  EXPECT_EQ(combobox_->GetSelectedItemIndex(), -1);
  combobox_->AddItem("item");
  combobox_->AddItem("item2");
  combobox_->AddItem("item3");
  EXPECT_EQ(combobox_->GetItems().size(), static_cast<size_t>(3));
  combobox_->RemoveItemAt(1);
  EXPECT_EQ(combobox_->GetItems().size(), static_cast<size_t>(2));
  combobox_->RemoveItemAt(1);
  EXPECT_EQ(combobox_->GetItems().size(), static_cast<size_t>(1));
}

TEST_F(ComboBoxTest, NoDefaultSelection) {
  combobox_->AddItem("item");
  EXPECT_EQ(combobox_->GetSelectedItemIndex(), -1);
}

TEST_F(ComboBoxTest, AllowDuplicate) {
  combobox_->AddItem("item");
  combobox_->AddItem("item");
  EXPECT_EQ(combobox_->GetItems().size(), static_cast<size_t>(2));
}

TEST_F(ComboBoxTest, SelectItemShouldNotEmitEvent) {
  combobox_->AddItem("item1");
  combobox_->AddItem("item2");
  bool emitted = false;
  combobox_->on_selection_change.Connect([&emitted](nu::Picker*) {
    emitted = true;
  });
  combobox_->SelectItemAt(1);
  EXPECT_FALSE(emitted);
  EXPECT_EQ(combobox_->GetText(), "item2");
}

TEST_F(ComboBoxTest, ChangeTextShouldNotEmitEvent) {
  bool emitted = false;
  combobox_->on_text_change.Connect([&emitted](nu::ComboBox*) {
    emitted = true;
  });
  combobox_->SetText("text");
  EXPECT_FALSE(emitted);
}
