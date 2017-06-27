// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/lifetime.h"

namespace nu {

void Lifetime::PlatformInit() {
}

void Lifetime::PlatformDestroy() {
}

void Lifetime::Run() {
  MSG msg;
  while (::GetMessage(&msg, NULL, 0, 0)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
  }
}

void Lifetime::Quit() {
  ::PostQuitMessage(0);
}

void Lifetime::PostTask(const base::Closure& task) {
}

void Lifetime::PostDelayedTask(int ms, const base::Closure& task) {
}

}  // namespace nu
