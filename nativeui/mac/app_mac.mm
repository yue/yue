// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/app.h"

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/geometry/safe_integer_conversions.h"
#include "nativeui/mac/nu_application_delegate.h"
#include "nativeui/menu_bar.h"

namespace nu {

Color App::PlatformGetColor(ThemeColor name) {
  NSColor* color;
  if (name == ThemeColor::Text)
    color = [NSColor textColor];
  else if (name == ThemeColor::DisabledText)
    color = [NSColor disabledControlTextColor];
  else
    color = [NSColor blackColor];
  CGFloat red, green, blue, alpha;
  color = [color colorUsingColorSpace:[NSColorSpace deviceRGBColorSpace]];
  [color getRed:&red green:&green blue:&blue alpha:&alpha];
  return Color(ToRoundedInt(255. * alpha),
               ToRoundedInt(255. * red),
               ToRoundedInt(255. * green),
               ToRoundedInt(255. * blue));
}

void App::SetApplicationMenu(MenuBar* menu) {
  application_menu_ = menu;
  [NSApp setMainMenu:menu->GetNative()];
}

MenuBar* App::GetApplicationMenu() const {
  return application_menu_.get();
}

void App::SetDockBadgeLabel(const std::string& text) {
  [[NSApp dockTile] setBadgeLabel:base::SysUTF8ToNSString(text)];
}

std::string App::GetDockBadgeLabel() const {
  return base::SysNSStringToUTF8([[NSApp dockTile] badgeLabel]);
}

}  // namespace nu
