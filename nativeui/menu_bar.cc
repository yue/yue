// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_bar.h"

namespace nu {

// static
const char MenuBar::kClassName[] = "MenuBar";

MenuBar::MenuBar() : MenuBase(PlatformCreate()) {
  SetAcceleratorManager(&accel_manager_);
}

const char* MenuBar::GetClassName() const {
  return kClassName;
}

}  // namespace nu
