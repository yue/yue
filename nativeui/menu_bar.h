// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MENU_BAR_H_
#define NATIVEUI_MENU_BAR_H_

#include "nativeui/menu_base.h"

namespace nu {

NATIVEUI_EXPORT class MenuBar : public MenuBase {
 public:
  MenuBar();

 protected:
  ~MenuBar() override = default;
};

}  // namespace nu

#endif  // NATIVEUI_MENU_BAR_H_
