// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "base/strings/stringprintf.h"
#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class TableTest : public testing::Test {
 protected:
  void SetUp() override {
    table_ = new nu::Table();
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::Table> table_;
};

TEST_F(TableTest, Empty) {
  EXPECT_EQ(table_->GetSelectedRow(), -1);
  EXPECT_EQ(table_->GetColumnCount(), 0);
}

TEST_F(TableTest, AddColumn) {
  table_->AddColumn("A");
  table_->AddColumn("B");
  EXPECT_EQ(table_->GetColumnCount(), 2);
}

class TestTableModel : public nu::TableModel {
 public:
  TestTableModel() {}

  uint32_t GetRowCount() const override {
    return 10000;
  }

  const base::Value* GetValue(uint32_t column, uint32_t row) const override {
    value_ = base::Value(base::StringPrintf("%d_%d", column, row));
    return &value_;
  }

  void SetValue(uint32_t column, uint32_t row, base::Value value) override {
  }

 private:
  ~TestTableModel() override {}

  mutable base::Value value_;
};

TEST_F(TableTest, SelectSingleRow) {
  table_->SetModel(new TestTableModel);
  table_->SelectRow(9999);
  EXPECT_EQ(table_->GetSelectedRow(), 9999);
  table_->SelectRow(100001);
  EXPECT_EQ(table_->GetSelectedRow(), 9999);
}
