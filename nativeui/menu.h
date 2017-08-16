// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MENU_H_
#define NATIVEUI_MENU_H_

#include "nativeui/menu_base.h"

namespace nu {

class NATIVEUI_EXPORT Menu : public MenuBase {
 public:
  Menu();

  // View class name.
  static const char kClassName[];

  // MenuBase:
  const char* GetClassName() const override;

  void Popup();

 protected:
  ~Menu() override = default;
};

}  // namespace nu

#endif  // NATIVEUI_MENU_H_
