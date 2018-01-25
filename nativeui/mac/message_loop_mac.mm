// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/message_loop.h"

#import <Cocoa/Cocoa.h>

namespace nu {

// static
void MessageLoop::Run() {
  [NSApp run];
}

// static
void MessageLoop::Quit() {
  [NSApp stop:nil];
  // Must post an event, otherwise an idle loop may not response to stop.
  [NSApp postEvent:[NSEvent otherEventWithType:NSApplicationDefined
                                      location:NSZeroPoint
                                 modifierFlags:0
                                     timestamp:0
                                  windowNumber:0
                                       context:NULL
                                       subtype:0
                                         data1:0
                                         data2:0]
           atStart:NO];
}

// static
void MessageLoop::PostTask(const std::function<void()>& task) {
  __block auto callback = task;
  dispatch_async(dispatch_get_main_queue(), ^{
    callback();
  });
}

// static
void MessageLoop::PostDelayedTask(int ms, const std::function<void()>& task) {
  __block std::function<void()> callback = task;
  dispatch_time_t t = dispatch_time(DISPATCH_TIME_NOW, ms * NSEC_PER_MSEC);
  dispatch_after(t, dispatch_get_main_queue(), ^{
    callback();
  });
}

}  // namespace nu
