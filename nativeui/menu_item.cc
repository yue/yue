// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_item.h"

namespace nu {

MenuItem::MenuItem(Type type) : type_(type) {
  PlatformInit();
}

MenuItem::~MenuItem() {
  PlatformDestroy();
}

}  // namespace nu
