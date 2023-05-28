// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu.h"

#import <Cocoa/Cocoa.h>

#include "nativeui/events/event.h"
#include "nativeui/gfx/mac/coordinate_conversion.h"

namespace nu {

Menu::Menu() : MenuBase([[NSMenu alloc] init]) {}

void Menu::Popup() {
  PopupAt(Event::GetMouseLocation());
}

void Menu::PopupAt(const PointF& point) {
  [GetNative() popUpMenuPositioningItem:nil
                             atLocation:ScreenPointToNSPoint(point)
                                 inView:nil];
}

}  // namespace nu
