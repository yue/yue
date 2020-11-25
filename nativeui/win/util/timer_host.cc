// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/util/timer_host.h"

#include <utility>

namespace nu {

TimerHost::TimerHost() {}

TimerHost::~TimerHost() {}

TimerHost::TimerId TimerHost::SetTimeout(int ms, Task task) {
  base::AutoLock auto_lock(lock_);
  TimerId id = NextTimerId();
  if (::SetTimer(hwnd(), id, ms, nullptr))
    tasks_[id] = std::move(task);
  return id;
}

void TimerHost::ClearTimeout(TimerId id) {
  ::KillTimer(hwnd(), id);
  base::AutoLock auto_lock(lock_);
  tasks_.erase(id);
}

void TimerHost::OnTimer(UINT_PTR id) {
  ::KillTimer(hwnd(), id);
  std::function<void()> task;
  {
    base::AutoLock auto_lock(lock_);
    auto it = tasks_.find(id);
    if (it == tasks_.end())
      return;
    task = std::move(it->second);
    tasks_.erase(it);
  }
  task();
}

UINT_PTR TimerHost::NextTimerId() {
  return static_cast<UINT_PTR>(++next_timer_id_);
}

}  // namespace nu
