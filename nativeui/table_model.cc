// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/table_model.h"

#include <utility>

namespace nu {

///////////////////////////////////////////////////////////////////////////////
// TableModel implementation.

TableModel::TableModel() {}

TableModel::~TableModel() {}

void TableModel::NotifyRowInsertion(size_t row) {
}

void TableModel::NotifyRowDeletion(size_t row) {
}

void TableModel::NotifyRowChange(size_t row) {
}

void TableModel::Subscribe(Table* view) {
  tables_.push_back(view);
}

void TableModel::Unsubscribe(Table* view) {
  tables_.remove(view);
}

///////////////////////////////////////////////////////////////////////////////
// AbstractTableModel implementation.

AbstractTableModel::AbstractTableModel() {}

AbstractTableModel::~AbstractTableModel() {}

size_t AbstractTableModel::GetRowCount() const {
  if (!get_row_count)
    return 0;
  return get_row_count(const_cast<AbstractTableModel*>(this));
}

const base::Value* AbstractTableModel::GetValue(
    Table* view, size_t column, size_t row) const {
  if (!get_value)
    return nullptr;
  return get_value(const_cast<AbstractTableModel*>(this), view, column, row);
}

///////////////////////////////////////////////////////////////////////////////
// SimpleTableModel implementation.

SimpleTableModel::SimpleTableModel(size_t columns) : columns_(columns) {}

SimpleTableModel::~SimpleTableModel() {}

void SimpleTableModel::AddRow(Row data) {
  if (data.size() == columns_)
    rows_.emplace_back(std::move(data));
}

void SimpleTableModel::RemoveRowAt(size_t row) {
  if (row > 0 && row < rows_.size())
    rows_.erase(rows_.begin() + row);
}

void SimpleTableModel::SetValue(size_t column, size_t row, base::Value value) {
  if (columns_ > 0 && column < columns_ && row > 0 && row < rows_.size())
    rows_[row][column] = std::move(value);
}

size_t SimpleTableModel::GetRowCount() const {
  return rows_.size();
}

const base::Value* SimpleTableModel::GetValue(
    Table* view, size_t column, size_t row) const {
  if (columns_ > 0 && column < columns_ && row > 0 && row < rows_.size())
    return &rows_[row][column];
  return nullptr;
}

}  // namespace nu
