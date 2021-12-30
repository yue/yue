// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_SCROLLBAR_REPEAT_CONTROLLER_H_
#define NATIVEUI_WIN_SCROLLBAR_REPEAT_CONTROLLER_H_

#include <functional>

#include "base/memory/weak_ptr.h"

namespace nu {

// An object that handles auto-repeating UI actions. There is a longer initial
// delay after which point repeats become constant. Users provide a callback
// that is notified when each repeat occurs so that they can perform the
// associated action.
class RepeatController {
 public:
  explicit RepeatController(const std::function<void()>& callback);
  virtual ~RepeatController();

  // Start repeating.
  void Start();

  // Stop repeating.
  void Stop();

 private:
  // Called when the timer expires.
  void Run();

  bool running_;
  std::function<void()> callback_;

  base::WeakPtrFactory<RepeatController> weak_factory_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_SCROLLBAR_REPEAT_CONTROLLER_H_
