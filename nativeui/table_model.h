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
#include "nativeui/nativeui_export.h"

namespace nu {

class Table;

// Users should sublcass TableModel to provide their own implementation.
class NATIVEUI_EXPORT TableModel : public base::RefCounted<TableModel> {
 public:
  // Return how many rows are in the model.
  virtual uint32_t GetRowCount() const = 0;

  // Return the reference to the data in the model.
  // Caller should not store the return value, as it is a temporary reference
  // that may immediately get destroyed after exiting current stack.
  virtual const base::Value* GetValue(uint32_t column, uint32_t row) const = 0;

  // Change the value.
  virtual void SetValue(uint32_t column, uint32_t row, base::Value value) = 0;

  // Called by sublcass to notify when there rows inserted.
  void NotifyRowInsertion(uint32_t row);
  void NotifyRowDeletion(uint32_t row);
  void NotifyValueChange(uint32_t column, uint32_t row);

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
class NATIVEUI_EXPORT AbstractTableModel : public TableModel {
 public:
  // TODO(zcbenz): Handle index_starts_from_0 in language bindings.
  explicit AbstractTableModel(bool index_starts_from_0 = true);

  // TableModel:
  uint32_t GetRowCount() const override;
  const base::Value* GetValue(uint32_t column, uint32_t row) const override;
  void SetValue(uint32_t column, uint32_t row, base::Value value) override;

  // Delegate methods.
  std::function<uint32_t(AbstractTableModel*)> get_row_count;
  std::function<base::Value(AbstractTableModel*, uint32_t, uint32_t)> get_value;
  std::function<void(AbstractTableModel*,
                     uint32_t, uint32_t, base::Value)> set_value;

 protected:
  ~AbstractTableModel() override;

 private:
  bool index_starts_from_0_;
  base::Value copy_;
};

// A simple implementation of TableModel that manages the data.
class NATIVEUI_EXPORT SimpleTableModel : public TableModel {
 public:
  using Row = std::vector<base::Value>;

  explicit SimpleTableModel(uint32_t columns);

  void AddRow(Row data);
  void RemoveRowAt(uint32_t row);

  // TableModel:
  uint32_t GetRowCount() const override;
  const base::Value* GetValue(uint32_t column, uint32_t row) const override;
  void SetValue(uint32_t column, uint32_t row, base::Value value) override;

 protected:
  ~SimpleTableModel() override;

 private:
  const uint32_t columns_;
  std::vector<Row> rows_;
};

}  // namespace nu

#endif  // NATIVEUI_TABLE_MODEL_H_
