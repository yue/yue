// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/mac/drag_drop/nested_run_loop.h"

#import <AppKit/AppKit.h>

#include "base/auto_reset.h"

namespace nu {

namespace {

void NoOp(void* info) {
}

}  // namespace

NestedRunLoop::NestedRunLoop() : run_loop_(CFRunLoopGetCurrent()) {
  CFRetain(run_loop_);

  CFRunLoopSourceContext source_context = CFRunLoopSourceContext();
  source_context.perform = NoOp;
  quit_source_ = CFRunLoopSourceCreate(nullptr, 0, &source_context);
  CFRunLoopAddSource(run_loop_, quit_source_, kCFRunLoopCommonModes);
}

NestedRunLoop::~NestedRunLoop() {
  CFRunLoopRemoveSource(run_loop_, quit_source_, kCFRunLoopCommonModes);
  CFRelease(quit_source_);
  CFRelease(run_loop_);
}

void NestedRunLoop::Run() {
  base::AutoReset<bool> auto_reset_keep_running(&keep_running_, true);

  while (keep_running_) {
    // NSRunLoop manages autorelease pools itself.
    [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode
                             beforeDate:[NSDate distantFuture]];
  }
}

void NestedRunLoop::Quit() {
  keep_running_ = false;
  CFRunLoopSourceSignal(quit_source_);
  CFRunLoopWakeUp(run_loop_);
}

}  // namespace nu
