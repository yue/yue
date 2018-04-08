// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/tray.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/image.h"
#include "nativeui/menu.h"

namespace nu {

Tray::Tray(Image* icon)
    : tray_([[[NSStatusBar systemStatusBar]
                statusItemWithLength:NSSquareStatusItemLength] retain]) {
  SetImage(icon);
}

Tray::Tray(const std::string& title)
    : tray_([[[NSStatusBar systemStatusBar]
                statusItemWithLength:NSVariableStatusItemLength] retain]) {
  SetTitle(title);
}

Tray::~Tray() {
  [tray_ release];
}

void Tray::SetTitle(const std::string& title) {
  [tray_ setTitle:base::SysUTF8ToNSString(title)];
}

void Tray::SetImage(Image* icon) {
  [tray_ setImage:icon->GetNative()];
}

void Tray::SetMenu(Menu* menu) {
  menu_ = menu;
  [tray_ setMenu:menu->GetNative()];
}

}  // namespace nu
