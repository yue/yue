// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/responder.h"

#include "nativeui/mac/events_handler.h"
#include "nativeui/mac/nu_view.h"

namespace nu {

void Responder::PlatformInstallMouseClickEvents() {
  AddMouseClickEventHandler(GetNative());
}

void Responder::PlatformInstallMouseMoveEvents() {
  [GetNative() enableTracking];
  AddMouseMoveEventHandler(GetNative());
}

void Responder::PlatformInstallKeyEvents() {
  AddKeyEventHandler(GetNative());
}

}  // namespace nu
