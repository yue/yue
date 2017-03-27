// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu.h"

#import <Cocoa/Cocoa.h>

namespace nu {

Menu::Menu() : MenuBase([[NSMenu alloc] init]) {
}

void Menu::Popup() {
  [GetNative() popUpMenuPositioningItem:nil
                        atLocation:[NSEvent mouseLocation]
                            inView:nil];
}

}  // namespace nu
