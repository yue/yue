// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/lifetime.h"

#include <iostream>

#include "nativeui/mac/nu_application_delegate.h"

namespace nu {

void Lifetime::PlatformInit() {
  DCHECK(![NSApp delegate])
      << "Lifetime should not be used in apps with their own message loop.";
  app_delegate_ = [[NUApplicationDelegate alloc] initWithShell:this];
  [NSApp setDelegate:app_delegate_];
}

void Lifetime::PlatformDestroy() {
  [NSApp setDelegate:nil];
  [app_delegate_ release];
}

}  // namespace nu
