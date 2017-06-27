// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/scrollbar/repeat_controller.h"

#include "base/bind.h"
#include "nativeui/lifetime.h"

namespace nu {

namespace {

// The delay before the first and then subsequent repeats. Values taken from
// XUL code: http://mxr.mozilla.org/seamonkey/source/layout/xul/base/src/nsRepeatService.cpp#52
const int kInitialRepeatDelay = 250;
const int kRepeatDelay = 50;

}  // namespace

RepeatController::RepeatController(const base::Closure& callback)
    : running_(false),
      lifetime_(Lifetime::GetCurrent()),
      callback_(callback),
      weak_factory_(this) {
}

RepeatController::~RepeatController() {
}

void RepeatController::Start() {
  running_ = true;
  lifetime_->PostDelayedTask(
      kInitialRepeatDelay,
      base::Bind(&RepeatController::Run, weak_factory_.GetWeakPtr()));
}

void RepeatController::Stop() {
  running_ = false;
}

void RepeatController::Run() {
  if (running_) {
    lifetime_->PostDelayedTask(
        kRepeatDelay,
        base::Bind(&RepeatController::Run, weak_factory_.GetWeakPtr()));
    callback_.Run();
  }
}

}  // namespace nu
