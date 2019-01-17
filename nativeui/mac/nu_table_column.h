// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_NU_TABLE_COLUMN_H_
#define NATIVEUI_MAC_NU_TABLE_COLUMN_H_

#import <Cocoa/Cocoa.h>

#include "nativeui/table.h"

@interface NUTableColumn : NSTableColumn
@property uint32_t columnInModel;
@property nu::Table::ColumnOptions options;

- (id)initWithTable:(nu::Table*)table
              title:(const std::string&)title
            options:(const nu::Table::ColumnOptions&)options;
@end

#endif  // NATIVEUI_MAC_NU_TABLE_COLUMN_H_
