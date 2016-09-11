// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/scroll_bar/repeat_controller.h"

namespace nu {

namespace {

// The delay before the first and then subsequent repeats. Values taken from
// XUL code: http://mxr.mozilla.org/seamonkey/source/layout/xul/base/src/nsRepeatService.cpp#52
const int kInitialRepeatDelay = 250;
const int kRepeatDelay = 50;

}  // namespace

using base::TimeDelta;

RepeatController::RepeatController(const base::Closure& callback)
    : callback_(callback) {
}

RepeatController::~RepeatController() {
}

void RepeatController::Start() {
  // The first timer is slightly longer than subsequent repeats.
  timer_.Start(FROM_HERE, TimeDelta::FromMilliseconds(kInitialRepeatDelay),
               this, &RepeatController::Run);
}

void RepeatController::Stop() {
  timer_.Stop();
}

void RepeatController::Run() {
  timer_.Start(FROM_HERE, TimeDelta::FromMilliseconds(kRepeatDelay), this,
               &RepeatController::Run);
  callback_.Run();
}

}  // namespace nu
