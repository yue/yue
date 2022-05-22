// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/responder.h"

namespace nu {

Responder::Responder() {
  // Lazy install event handlers.
  on_mouse_down.SetDelegate(this, kOnMouseClick);
  on_mouse_up.SetDelegate(this, kOnMouseClick);
  on_mouse_move.SetDelegate(this, kOnMouseMove);
  on_mouse_enter.SetDelegate(this, kOnMouseMove);
  on_mouse_leave.SetDelegate(this, kOnMouseMove);
  on_key_down.SetDelegate(this, kOnKey);
  on_key_up.SetDelegate(this, kOnKey);
}

Responder::~Responder() = default;

void Responder::InitResponder(NativeResponder native, Type type) {
  responder_ = native;
  type_ = type;
}

void Responder::OnConnect(int identifier) {
  switch (identifier) {
    case kOnMouseClick:
      if (!on_mouse_click_installed_) {
        PlatformInstallMouseClickEvents();
        on_mouse_click_installed_ = true;
      }
      break;
    case kOnMouseMove:
      if (!on_mouse_move_installed_) {
        PlatformInstallMouseMoveEvents();
        on_mouse_move_installed_ = true;
      }
      break;
    case kOnKey:
      if (!on_key_installed_) {
        PlatformInstallKeyEvents();
        on_key_installed_ = true;
      }
      break;
  }
}

}  // namespace nu
