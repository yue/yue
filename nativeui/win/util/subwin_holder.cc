// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/util/subwin_holder.h"

#include "nativeui/win/subwin_view.h"
#include "nativeui/win/util/hwnd_util.h"

namespace nu {

SubwinHolder::SubwinHolder() {
}

SubwinHolder::~SubwinHolder() {
}

void SubwinHolder::OnCommand(UINT code, int command, HWND window) {
  if (!window)
    return;
  auto* control = reinterpret_cast<SubwinView*>(GetWindowUserData(window));
  control->OnCommand(code, command);
}

}  // namespace nu
