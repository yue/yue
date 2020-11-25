// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_UTIL_TIMER_HOST_H_
#define NATIVEUI_WIN_UTIL_TIMER_HOST_H_

#include <functional>
#include <unordered_map>

#include "base/synchronization/lock.h"
#include "nativeui/win/util/win32_window.h"

namespace nu {

class TimerHost : public Win32Window {
 public:
  using Task = std::function<void()>;
  using TimerId = UINT_PTR;

  TimerHost();
  ~TimerHost() override;

  TimerId SetTimeout(int ms, Task task);
  void ClearTimeout(TimerId id);

 protected:
  CR_BEGIN_MSG_MAP_EX(TimerHost, Win32Window)
    CR_MSG_WM_TIMER(OnTimer)
  CR_END_MSG_MAP()

  void OnTimer(UINT_PTR id);

 private:
  UINT_PTR NextTimerId();

  // The unique timer ID we will assign to the next timer.
  UINT next_timer_id_ = 0;

  base::Lock lock_;
  std::unordered_map<TimerId, Task> tasks_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_UTIL_TIMER_HOST_H_
