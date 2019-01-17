// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_NU_TABLE_DATA_SOURCE_H_
#define NATIVEUI_MAC_NU_TABLE_DATA_SOURCE_H_

#import <Cocoa/Cocoa.h>

#include "nativeui/table_model.h"

@interface NUTableDataSource : NSObject<NSTableViewDataSource> {
 @private
  scoped_refptr<nu::TableModel> model_;
}
- (id)initWithTableModel:(nu::TableModel*)model;
@end

#endif  // NATIVEUI_MAC_NU_TABLE_DATA_SOURCE_H_
