// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/table/nu_table_column.h"
#include "nativeui/mac/table/nu_table_data_source.h"
#include "nativeui/mac/table/nu_wrapped_value.h"

@implementation NUTableDataSource

- (id)initWithTableModel:(nu::TableModel*)model {
  if ((self = [super init]))
    model_ = model;
  return self;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView*)tableView {
  return model_->GetRowCount();
}

- (id)tableView:(NSTableView*)tableView
    objectValueForTableColumn:(NSTableColumn*)nsTableColumn
                          row:(NSInteger)row {
  auto* tableColumn = static_cast<NUTableColumn*>(nsTableColumn);
  return [NUWrappedValue
      valueWithValue:model_->GetValue(tableColumn.columnInModel, row)];
}

@end
