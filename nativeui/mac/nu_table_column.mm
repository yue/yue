// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/nu_table_column.h"

#include "base/strings/sys_string_conversions.h"

namespace {

// Increasing ID for table column identifier.
int g_next_id = 0;

}  // namespace

@implementation NUTableColumn

@synthesize columnInModel;
@synthesize options;

- (id)initWithTable:(nu::Table*)table
              title:(const std::string&)title
            options:(const nu::Table::ColumnOptions&)columnOptions {
  NSString* identifier = [NSString stringWithFormat:@"%d", ++g_next_id];
  if ((self = [super initWithIdentifier:identifier])) {
    self.columnInModel = options.column == -1 ? table->GetColumnCount()
                                              : columnOptions.column;
    self.options = columnOptions;
    self.headerCell.stringValue = base::SysUTF8ToNSString(title);

    // Handle width property.
    NSTableColumnResizingOptions resizingMask = NSTableColumnUserResizingMask;
    if (columnOptions.width < 0)
      resizingMask |= NSTableColumnAutoresizingMask;
    else
      self.width = columnOptions.width;
    self.resizingMask = resizingMask;
  }
  return self;
}

@end
