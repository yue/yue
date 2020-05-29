// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/app.h"

#include "base/strings/sys_string_conversions.h"
#include "nativeui/mac/nu_application_delegate.h"
#include "nativeui/menu_bar.h"

namespace nu {

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

void App::Activate(bool force) {
  [NSApp activateIgnoringOtherApps:force];
}

void App::Deactivate() {
  [NSApp deactivate];
}

bool App::IsActive() const {
  return [NSApp isActive];
}

void App::SetActivationPolicy(ActivationPolicy policy) {
  NSApplicationActivationPolicy p = NSApplicationActivationPolicyRegular;
  switch (policy) {
    case ActivationPolicy::Regular:
      p = NSApplicationActivationPolicyRegular;
      break;
    case ActivationPolicy::Accessory:
      p = NSApplicationActivationPolicyAccessory;
      break;
    case ActivationPolicy::Prohibited:
      p = NSApplicationActivationPolicyProhibited;
      break;
  }
  [NSApp setActivationPolicy:p];
}

App::ActivationPolicy App::GetActivationPolicy() const {
  switch ([NSApp activationPolicy]) {
    case NSApplicationActivationPolicyRegular:
      return ActivationPolicy::Regular;
    case NSApplicationActivationPolicyAccessory:
      return ActivationPolicy::Accessory;
    case NSApplicationActivationPolicyProhibited:
      return ActivationPolicy::Prohibited;
  }
}

}  // namespace nu
