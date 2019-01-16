// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_TABLE_MODEL_H_
#define NATIVEUI_TABLE_MODEL_H_

#include <functional>
#include <list>
#include <vector>

#include "base/memory/ref_counted.h"
#include "base/values.h"

namespace nu {

class Table;

// Users should sublcass TableModel to provide their own implementation.
class TableModel : public base::RefCounted<TableModel> {
 public:
  // Must be overridden by sublcass.
  virtual size_t GetRowCount() const = 0;
  virtual const base::Value* GetValue(
      Table* view, size_t column, size_t row) const = 0;

  // Called by sublcass to notify when there rows inserted.
  void NotifyRowInsertion(size_t row);
  void NotifyRowDeletion(size_t row);
  void NotifyRowChange(size_t row);

 protected:
  TableModel();
  virtual ~TableModel();

 private:
  friend class base::RefCounted<TableModel>;
  friend class Table;

  // Called by table.
  void Subscribe(Table* view);
  void Unsubscribe(Table* view);

  std::list<Table*> tables_;
};

// Used by language bindings.
class AbstractTableModel : public TableModel {
 public:
  AbstractTableModel();

  // TableModel:
  size_t GetRowCount() const override;
  const base::Value* GetValue(
      Table* view, size_t column, size_t row) const override;

  // Delegate methods.
  std::function<size_t(AbstractTableModel*)> get_row_count;
  std::function<
    const base::Value*(AbstractTableModel*,
                       Table* view, size_t column, size_t row)> get_value;

 protected:
  ~AbstractTableModel() override;
};

// A simple implementation of TableModel that manages the data.
class SimpleTableModel : public TableModel {
 public:
  using Row = std::vector<base::Value>;

  explicit SimpleTableModel(size_t columns);

  void AddRow(Row data);
  void RemoveRowAt(size_t row);
  void SetValue(size_t column, size_t row, base::Value value);

  // TableModel:
  size_t GetRowCount() const override;
  const base::Value* GetValue(
      Table* view, size_t column, size_t row) const override;

 protected:
  ~SimpleTableModel() override;

 private:
  const size_t columns_;
  std::vector<Row> rows_;
};

}  // namespace nu

#endif  // NATIVEUI_TABLE_MODEL_H_
