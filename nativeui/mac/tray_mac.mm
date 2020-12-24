// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/tray.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/geometry/rect_f.h"
#include "nativeui/gfx/image.h"
#include "nativeui/gfx/mac/coordinate_conversion.h"
#include "nativeui/menu.h"

namespace nu {

Tray::Tray(scoped_refptr<Image> icon)
    : tray_([[[NSStatusBar systemStatusBar]
                statusItemWithLength:NSSquareStatusItemLength] retain]) {
  SetImage(std::move(icon));
}

Tray::Tray(const std::string& title)
    : tray_([[[NSStatusBar systemStatusBar]
                statusItemWithLength:NSVariableStatusItemLength] retain]) {
  SetTitle(title);
}

Tray::~Tray() {
  Remove();  // macOS does not remove the icon when obj is destroyed
  [tray_ release];
}

void Tray::Remove() {
  [[NSStatusBar systemStatusBar] removeStatusItem:tray_];
}

RectF Tray::GetBounds() const {
  NSStatusBarButton* button = [tray_ button];
  NSRect rect = [button convertRect:[button bounds] toView:nil];
  return ScreenRectFromNSRect([[button window] convertRectToScreen:rect]);
}

void Tray::SetTitle(const std::string& title) {
  [tray_ setTitle:base::SysUTF8ToNSString(title)];
}

void Tray::SetImage(scoped_refptr<Image> icon) {
  [tray_ setImage:icon->GetNative()];
}

void Tray::SetMenu(scoped_refptr<Menu> menu) {
  [tray_ setMenu:menu->GetNative()];
  menu_ = std::move(menu);
}

}  // namespace nu
