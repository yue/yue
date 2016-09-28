// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/app.h"

#include "nativeui/mac/nu_application_delegate.h"
#include "nativeui/menu_bar.h"

namespace nu {

void App::PlatformInit() {
  DCHECK(![NSApp delegate]);
  app_delegate_ = [[NUApplicationDelegate alloc] initWithShell:this];
  [NSApp setDelegate:app_delegate_];
}

void App::PlatformDestroy() {
  [NSApp setDelegate:nil];
  [app_delegate_ release];
}

void App::PlatformSetApplicationMenu(MenuBar* menu) {
  [NSApp setMainMenu:menu->menu()];
}

}  // namespace nu
