// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/lifetime.h"

#include "base/lazy_instance.h"

namespace nu {

// static
base::Lock Lifetime::lock_;

// static
std::unordered_map<UINT_PTR, base::Closure> Lifetime::tasks_;

// static
void Lifetime::OnTimer(HWND, UINT, UINT_PTR event, DWORD) {
  ::KillTimer(NULL, event);
  base::AutoLock auto_lock(lock_);
  tasks_[event].Run();
  tasks_.erase(event);
}

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
  PostDelayedTask(USER_TIMER_MINIMUM, task);
}

void Lifetime::PostDelayedTask(int ms, const base::Closure& task) {
  UINT_PTR event = ::SetTimer(NULL, NULL, ms, OnTimer);
  base::AutoLock auto_lock(lock_);
  tasks_[event] = task;
}

}  // namespace nu
