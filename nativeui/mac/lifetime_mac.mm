// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/lifetime.h"

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

void Lifetime::Run() {
  [NSApp run];
}

void Lifetime::Quit() {
  [NSApp stop];
}

void Lifetime::PostTask(const base::Closure& task) {
}

void Lifetime::PostDelayedTask(int ms, const base::Closure& task) {
}

}  // namespace nu
