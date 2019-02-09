// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_DRAG_DROP_DATA_PROVIDER_H_
#define NATIVEUI_MAC_DRAG_DROP_DATA_PROVIDER_H_

#import <Cocoa/Cocoa.h>

#include "nativeui/clipboard.h"

@interface DataProvider: NSObject<NSPasteboardItemDataProvider>
- (instancetype)initWithData:(std::vector<nu::Clipboard::Data>)objects;
- (NSPasteboard*)pasteboard;
@end

#endif  // NATIVEUI_MAC_DRAG_DROP_DATA_PROVIDER_H_
