// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/message_loop.h"

#include <windows.h>

namespace nu {

// static
base::Lock MessageLoop::lock_;

// static
std::unordered_map<MessageLoop::TimerId, MessageLoop::Task> MessageLoop::tasks_;

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
  UINT_PTR event = ::SetTimer(NULL, NULL, ms, OnTimer);
  base::AutoLock auto_lock(lock_);
  tasks_[event] = task;
  return event;
}

// static
void MessageLoop::ClearTimeout(TimerId id) {
  ::KillTimer(NULL, id);
  base::AutoLock auto_lock(lock_);
  tasks_.erase(id);
}

// static
void CALLBACK MessageLoop::OnTimer(HWND, UINT, UINT_PTR event, DWORD) {
  ::KillTimer(NULL, event);
  std::function<void()> task;
  {
    base::AutoLock auto_lock(lock_);
    auto it = tasks_.find(event);
    if (it == tasks_.end())  // could it happen?
      return;
    task = it->second;
    tasks_.erase(it);
  }
  task();
}

}  // namespace nu
