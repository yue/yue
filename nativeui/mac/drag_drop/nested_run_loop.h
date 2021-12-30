// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_MAC_DRAG_DROP_NESTED_RUN_LOOP_H_
#define NATIVEUI_MAC_DRAG_DROP_NESTED_RUN_LOOP_H_

#include <CoreFoundation/CoreFoundation.h>

namespace nu {

// Enter a nested run loop.
class NestedRunLoop {
 public:
  NestedRunLoop();
  ~NestedRunLoop();

  NestedRunLoop& operator=(const NestedRunLoop&) = delete;
  NestedRunLoop(const NestedRunLoop&) = delete;

  void Run();
  void Quit();

 private:
  // The thread's run loop.
  CFRunLoopRef run_loop_;

  // A source that doesn't do anything but provide something signalable
  // attached to the run loop.  This source will be signalled when Quit
  // is called, to cause the loop to wake up so that it can stop.
  CFRunLoopSourceRef quit_source_;

  // False after Quit is called.
  bool keep_running_ = false;
};

}  // namespace nu

#endif  // NATIVEUI_MAC_DRAG_DROP_NESTED_RUN_LOOP_H_
