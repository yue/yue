// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_NU_TABLE_CELL_H_
#define NATIVEUI_MAC_NU_TABLE_CELL_H_

#import <Cocoa/Cocoa.h>

#include "nativeui/table.h"

@interface NUTableCell : NSTableCellView {
 @private
  nu::Table::ColumnType type_;
  nu::TableModel* model_;  // weak ptr
  uint32_t column_;
  uint32_t row_;
}
- (id)initWithColumnOptions:(const nu::Table::ColumnOptions&)options;
- (void)setTableModel:(nu::TableModel*)model
               column:(uint32_t)column
                  row:(uint32_t)row;
@end

#endif  // NATIVEUI_MAC_NU_TABLE_CELL_H_
