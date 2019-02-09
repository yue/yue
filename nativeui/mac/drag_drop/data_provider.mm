// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/drag_drop/data_provider.h"

#include "nativeui/mac/drag_drop/unique_pasteboard.h"

using Data = nu::Clipboard::Data;

@implementation DataProvider {
  nu::UniquePasteboard pasteboard_;
}

- (instancetype)initWithData:(std::vector<Data>)objects {
  if ((self = [super init])) {
    nu::Clipboard(pasteboard_.get()).SetData(std::move(objects));
  }
  return self;
}

- (NSPasteboard*)pasteboard {
  return pasteboard_.get();
}

- (void)pasteboard:(NSPasteboard*)sender
                  item:(NSPasteboardItem*)item
    provideDataForType:(NSPasteboardType)type {
  [sender setData:[pasteboard_.get() dataForType:type] forType:type];
}

@end
