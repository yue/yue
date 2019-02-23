// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/table_model.h"

#include <utility>

#include "nativeui/table.h"

namespace nu {

///////////////////////////////////////////////////////////////////////////////
// TableModel implementation.

TableModel::TableModel() {}

TableModel::~TableModel() {}

void TableModel::NotifyRowInsertion(uint32_t row) {
  for (Table* table : tables_)
    table->NotifyRowInsertion(row);
}

void TableModel::NotifyRowDeletion(uint32_t row) {
  for (Table* table : tables_)
    table->NotifyRowDeletion(row);
}

void TableModel::NotifyValueChange(uint32_t column, uint32_t row) {
  for (Table* table : tables_)
    table->NotifyValueChange(column, row);
}

void TableModel::Subscribe(Table* view) {
  tables_.push_back(view);
}

void TableModel::Unsubscribe(Table* view) {
  tables_.remove(view);
}

///////////////////////////////////////////////////////////////////////////////
// AbstractTableModel implementation.

AbstractTableModel::AbstractTableModel(bool index_starts_from_0)
    : index_starts_from_0_(index_starts_from_0) {}

AbstractTableModel::~AbstractTableModel() {}

uint32_t AbstractTableModel::GetRowCount() const {
  if (!get_row_count)
    return 0;
  return get_row_count(const_cast<AbstractTableModel*>(this));
}

const base::Value* AbstractTableModel::GetValue(
    uint32_t column, uint32_t row) const {
  if (!get_value)
    return nullptr;
  if (!index_starts_from_0_) {
    column += 1;
    row += 1;
  }
  // We can not get a reference from scripting languages, so we just store a
  // temporary copy and return a reference to the copy.
  auto* self = const_cast<AbstractTableModel*>(this);
  self->copy_ = get_value(self, column, row);
  return &copy_;
}

void AbstractTableModel::SetValue(uint32_t column, uint32_t row,
                                  base::Value value) {
  if (!set_value)
    return;
  if (!index_starts_from_0_) {
    column += 1;
    row += 1;
  }
  set_value(const_cast<AbstractTableModel*>(this),
            column, row, std::move(value));
}

///////////////////////////////////////////////////////////////////////////////
// SimpleTableModel implementation.

SimpleTableModel::SimpleTableModel(uint32_t columns) : columns_(columns) {}

SimpleTableModel::~SimpleTableModel() {}

void SimpleTableModel::AddRow(Row data) {
  if (data.size() >= columns_) {
    rows_.emplace_back(std::move(data));
    NotifyRowInsertion(static_cast<uint32_t>(rows_.size()) - 1);
  } else {
    LOG(ERROR) << "AddRow failed because row length is less than column size.";
  }
}

void SimpleTableModel::RemoveRowAt(uint32_t row) {
  if (row >= 0 && row < rows_.size()) {
    rows_.erase(rows_.begin() + row);
    NotifyRowDeletion(row);
  } else {
    LOG(ERROR) << "RemoveRow failed because row index is not in model.";
  }
}

uint32_t SimpleTableModel::GetRowCount() const {
  return static_cast<uint32_t>(rows_.size());
}

const base::Value* SimpleTableModel::GetValue(
    uint32_t column, uint32_t row) const {
  if (columns_ >= 0 && column < columns_ && row >= 0 && row < rows_.size())
    return &rows_[row][column];
  return nullptr;
}

void SimpleTableModel::SetValue(uint32_t column, uint32_t row,
                                base::Value value) {
  if (columns_ >= 0 && column < columns_ && row >= 0 && row < rows_.size()) {
    rows_[row][column] = std::move(value);
    NotifyValueChange(column, row);
  }
}

}  // namespace nu
