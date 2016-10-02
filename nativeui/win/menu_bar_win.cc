// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_bar.h"

namespace nu {

NativeMenu MenuBar::PlatformCreate() const {
  return CreateMenu();
}

}  // namespace nu
