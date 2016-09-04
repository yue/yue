// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/subwin_holder.h"

#include "base/memory/singleton.h"
#include "nativeui/win/subwin_view.h"
#include "nativeui/win/util/hwnd_util.h"

namespace nu {

// static
SubwinHolder* SubwinHolder::GetInstance() {
  return base::Singleton<SubwinHolder>::get();
}

SubwinHolder::SubwinHolder() {
}

SubwinHolder::~SubwinHolder() {
}

void SubwinHolder::OnCommand(UINT code, int command, HWND window) {
  auto* control = reinterpret_cast<SubwinView*>(GetWindowUserData(window));
  control->OnCommand(code, command);
}

}  // namespace nu
