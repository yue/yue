// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MENU_BAR_H_
#define NATIVEUI_MENU_BAR_H_

#include "nativeui/accelerator_manager.h"
#include "nativeui/menu_base.h"

namespace nu {

class Window;

class NATIVEUI_EXPORT MenuBar : public MenuBase {
 public:
  MenuBar();

  // View class name.
  static const char kClassName[];

  // MenuBase:
  const char* GetClassName() const override;

  // Internal: Relationship with the window.
  void SetWindow(Window* window) { window_ = window; }
  Window* GetWindow() const { return window_; }

  // Internal: The AcceleratorManager managed by the MenuBar.
  AcceleratorManager* accel_manager() { return &accel_manager_; }

 private:
  ~MenuBar() override = default;

  NativeMenu PlatformCreate() const;

  Window* window_ = nullptr;
  AcceleratorManager accel_manager_;
};

}  // namespace nu

#endif  // NATIVEUI_MENU_BAR_H_
