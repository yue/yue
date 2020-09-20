// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_TABLE_H_
#define NATIVEUI_TABLE_H_

#include <string>

#include "nativeui/view.h"

namespace base {
class Value;
}

namespace nu {

class Painter;
class TableModel;

class NATIVEUI_EXPORT Table : public View {
 public:
  enum class ColumnType {
    Text,
    Edit,
    Custom,
  };

  struct NATIVEUI_EXPORT ColumnOptions {
    ColumnOptions();
    ColumnOptions(const ColumnOptions& other);
    ~ColumnOptions();

    ColumnType type = ColumnType::Text;
    // Method used for drawing the column when type is Custom.
    std::function<void(Painter*, const RectF&, const base::Value&)> on_draw;
    // Which column of model, -1 means current last column.
    int column = -1;
    // Initial width.
    int width = -1;
  };

  Table();

  // View class name.
  static const char kClassName[];

  void SetModel(scoped_refptr<TableModel> model);
  TableModel* GetModel();
  void AddColumn(const std::string& title);
  void AddColumnWithOptions(const std::string& title,
                            const ColumnOptions& options);
  int GetColumnCount() const;
  void SetColumnsVisible(bool visible);
  bool IsColumnsVisible() const;
  void SetRowHeight(float height);
  float GetRowHeight() const;
  void SelectRow(int row);
  int GetSelectedRow() const;

  // View:
  const char* GetClassName() const override;

 protected:
  ~Table() override;

  NativeView PlatformCreate();
  void PlatformDestroy();
  void PlatformSetModel(TableModel* model);

 private:
  friend class TableModel;

  // Called by TableModel.
  void NotifyRowInsertion(uint32_t row);
  void NotifyRowDeletion(uint32_t row);
  void NotifyValueChange(uint32_t column, uint32_t row);

  scoped_refptr<TableModel> model_;
};

}  // namespace nu

#endif  // NATIVEUI_TABLE_H_
