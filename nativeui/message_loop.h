// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MESSAGE_LOOP_H_
#define NATIVEUI_MESSAGE_LOOP_H_

#include <unordered_map>

#include "base/synchronization/lock.h"
#include "nativeui/nativeui_export.h"

namespace nu {

// Communicate with the GUI message loop. All methods are thread-safe.
class NATIVEUI_EXPORT MessageLoop {
 public:
  // Function type for tasks.
  using Task = std::function<void()>;

  // Control message loop.
  static void Run();
  static void Quit();
  static void PostTask(const Task& task);
  static void PostDelayedTask(int ms, const Task& task);

 private:
#if defined(OS_WIN)
  static void CALLBACK OnTimer(HWND, UINT, UINT_PTR event, DWORD);

  static base::Lock lock_;
  static std::unordered_map<UINT_PTR, Task> tasks_;
#endif

  DISALLOW_IMPLICIT_CONSTRUCTORS(MessageLoop);
};

}  // namespace nu

#endif  // NATIVEUI_MESSAGE_LOOP_H_
