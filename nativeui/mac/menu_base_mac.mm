// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_base.h"

#import <Cocoa/Cocoa.h>

#include "nativeui/menu_item.h"

namespace nu {

void MenuBase::PlatformInit() {
}

void MenuBase::PlatformDestroy() {
  [menu_ release];
}

void MenuBase::PlatformInsert(MenuItem* item, int index) {
  [menu_ insertItem:item->GetNative() atIndex:index];
}

void MenuBase::PlatformRemove(MenuItem* item) {
  [menu_ removeItem:item->GetNative()];
}

}  // namespace nu
