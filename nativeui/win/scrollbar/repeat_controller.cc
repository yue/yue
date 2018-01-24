// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/scrollbar/repeat_controller.h"

#include "nativeui/message_loop.h"

namespace nu {

namespace {

// The delay before the first and then subsequent repeats. Values taken from
// XUL code: http://mxr.mozilla.org/seamonkey/source/layout/xul/base/src/nsRepeatService.cpp#52
const int kInitialRepeatDelay = 250;
const int kRepeatDelay = 50;

}  // namespace

RepeatController::RepeatController(const std::function<void()>& callback)
    : running_(false),
      callback_(callback),
      weak_factory_(this) {
}

RepeatController::~RepeatController() {
}

void RepeatController::Start() {
  running_ = true;
  auto self = weak_factory_.GetWeakPtr();
  MessageLoop::PostDelayedTask(kInitialRepeatDelay, [self] {
    if (self)
      self->Run();
  });
}

void RepeatController::Stop() {
  running_ = false;
}

void RepeatController::Run() {
  if (running_) {
    auto self = weak_factory_.GetWeakPtr();
    MessageLoop::PostDelayedTask(kRepeatDelay, [self] {
      if (self) {
        self->callback_();
        self->Run();
      }
    });
  }
}

}  // namespace nu
