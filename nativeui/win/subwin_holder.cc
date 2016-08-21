// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/subwin_holder.h"

#include "base/memory/singleton.h"

namespace nu {

// static
SubwinHolder* SubwinHolder::GetInstance() {
  return base::Singleton<SubwinHolder>::get();
}

SubwinHolder::SubwinHolder() {
}

SubwinHolder::~SubwinHolder() {
}

}  // namespace nu
