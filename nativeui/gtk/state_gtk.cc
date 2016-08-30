// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/state.h"

namespace nu {

void State::PlatformInit() {
  gtk_init(nullptr, nullptr);
}

void State::PlatformDestroy() {
}

}  // namespace nu
