// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/message_loop.h"

#import <Cocoa/Cocoa.h>
#import <CoreFoundation/CoreFoundation.h>

namespace nu {

namespace {

unsigned int g_task_id = 0;

}  // namespace

// static
base::Lock MessageLoop::lock_;

// static
std::unordered_map<MessageLoop::TimerId, MessageLoop::Task> MessageLoop::tasks_;

// static
void MessageLoop::Run() {
  [NSApp run];
}

// static
void MessageLoop::Quit() {
  [NSApp stop:nil];
  // Must post an event, otherwise an idle loop may not response to stop.
  [NSApp postEvent:[NSEvent otherEventWithType:NSEventTypeApplicationDefined
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
void MessageLoop::PostTask(Task task) {
  __block Task callback = std::move(task);
  dispatch_async(dispatch_get_main_queue(), ^{
    callback();
  });
}

// static
void MessageLoop::PostDelayedTask(int ms, Task task) {
  __block Task callback = std::move(task);
  dispatch_time_t t = dispatch_time(DISPATCH_TIME_NOW, ms * NSEC_PER_MSEC);
  dispatch_after(t, dispatch_get_main_queue(), ^{
    callback();
  });
}


//Enqueue a task to run multiple times
void MessageLoop::EnqueueTask(RepeatedTask task)
{ EnqueueDelayedTask(1, std::move(task)); }

void MessageLoop::EnqueueDelayedTask(int ms, RepeatedTask task)
{
    RepeatedTask callback = std::move(task);
    CFRunLoopRef run_loop = NSRunLoop.mainRunLoop.getCFRunLoop;
    CFRunLoopTimerContext context = { .info = new RepeatedTask(std::move(callback)) };
    CFRunLoopTimerRef timer = CFRunLoopTimerCreate(nullptr, 0, static_cast<float>(ms)/100, 0, 0, [](CFRunLoopTimerRef timer, void *ptr) {
        RepeatedTask callback = std::move(*static_cast<RepeatedTask *>(ptr));
        if (!callback()) {
          delete static_cast<RepeatedTask *>(ptr);
          CFRunLoopTimerInvalidate(timer);
        }
    }, &context);

    CFRunLoopAddTimer(run_loop, timer, kCFRunLoopCommonModes);
    CFRelease(timer);
}

// static
MessageLoop::TimerId MessageLoop::SetTimeout(int ms, Task task) {
  // Store the callback.
  __block TimerId id;
  {
    base::AutoLock auto_lock(lock_);
    id = ++g_task_id;
    tasks_[id] = std::move(task);
  }
  // Schedule a task to run the callback.
  dispatch_time_t t = dispatch_time(DISPATCH_TIME_NOW, ms * NSEC_PER_MSEC);
  dispatch_after(t, dispatch_get_main_queue(), ^{
    Task moved_task;
    {
      base::AutoLock auto_lock(lock_);
      auto it = tasks_.find(id);
      if (it == tasks_.end())  // cleared
        return;
      moved_task = std::move(it->second);
      tasks_.erase(it);
    }
    moved_task();
  });
  return id;
}

// static
void MessageLoop::ClearTimeout(TimerId id) {
  base::AutoLock auto_lock(lock_);
  tasks_.erase(id);
}

}  // namespace nu
