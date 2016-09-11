// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_SCROLL_BAR_REPEAT_CONTROLLER_H_
#define NATIVEUI_WIN_SCROLL_BAR_REPEAT_CONTROLLER_H_

#include "base/callback.h"
#include "base/macros.h"
#include "base/timer/timer.h"

namespace nu {

// An object that handles auto-repeating UI actions. There is a longer initial
// delay after which point repeats become constant. Users provide a callback
// that is notified when each repeat occurs so that they can perform the
// associated action.
class RepeatController {
 public:
  explicit RepeatController(const base::Closure& callback);
  virtual ~RepeatController();

  // Start repeating.
  void Start();

  // Stop repeating.
  void Stop();

 private:
  // Called when the timer expires.
  void Run();

  // The current timer.
  base::OneShotTimer timer_;

  base::Closure callback_;

  DISALLOW_COPY_AND_ASSIGN(RepeatController);
};

}  // namespace nu

#endif  // NATIVEUI_WIN_SCROLL_BAR_REPEAT_CONTROLLER_H_
