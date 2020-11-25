// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/message_loop.h"

#include <windows.h>

#include "nativeui/state.h"
#include "nativeui/win/util/timer_host.h"

namespace nu {

// static
void MessageLoop::Run() {
  MSG msg;
  while (::GetMessage(&msg, NULL, 0, 0)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
  }
}

// static
void MessageLoop::Quit() {
  ::PostQuitMessage(0);
}

// static
void MessageLoop::PostTask(const std::function<void()>& task) {
  PostDelayedTask(USER_TIMER_MINIMUM, task);
}

// static
void MessageLoop::PostDelayedTask(int ms, const std::function<void()>& task) {
  SetTimeout(ms, task);
}

// static
UINT_PTR MessageLoop::SetTimeout(int ms, const Task& task) {
  return State::GetMain()->GetTimerHost()->SetTimeout(ms, task);
}

// static
void MessageLoop::ClearTimeout(TimerId id) {
  State::GetMain()->GetTimerHost()->ClearTimeout(id);
}

}  // namespace nu
