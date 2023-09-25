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
    timeouts_[id] = std::move(task);
  return id;
}

void TimerHost::ClearTimeout(TimerId id) {
  base::AutoLock auto_lock(lock_);
  if (timeouts_.erase(id) > 0)
    ::KillTimer(hwnd(), id);
}

TimerHost::TimerId TimerHost::SetInterval(int ms, RepeatedTask task) {
  base::AutoLock auto_lock(lock_);
  TimerId id = NextTimerId();
  if (::SetTimer(hwnd(), id, ms, nullptr))
    intervals_[id] = std::move(task);
  return id;
}

void TimerHost::ClearInterval(TimerId id) {
  base::AutoLock auto_lock(lock_);
  if (intervals_.erase(id) > 0)
    ::KillTimer(hwnd(), id);
}

void TimerHost::OnTimer(UINT_PTR id) {
  // First search for timeouts.
  Task task;
  {
    base::AutoLock auto_lock(lock_);
    auto it = timeouts_.find(id);
    if (it != timeouts_.end()) {
      task = std::move(it->second);
      timeouts_.erase(it);
    }
  }
  if (task) {
    ::KillTimer(hwnd(), id);
    task();
    return;
  }
  // Then search for intervals.
  RepeatedTask repeated_task;
  {
    base::AutoLock auto_lock(lock_);
    auto it = intervals_.find(id);
    if (it != intervals_.end())
      repeated_task = it->second;
  }
  if (repeated_task && !repeated_task()) {
    ::KillTimer(hwnd(), id);
    base::AutoLock auto_lock(lock_);
    intervals_.erase(id);
  }
}

UINT_PTR TimerHost::NextTimerId() {
  return static_cast<UINT_PTR>(++next_timer_id_);
}

}  // namespace nu
