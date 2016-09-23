// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MENU_BASE_H_
#define NATIVEUI_MENU_BASE_H_

#include "base/memory/ref_counted.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"

namespace nu {

NATIVEUI_EXPORT class MenuBase : public base::RefCounted<MenuBase> {
 protected:
  explicit MenuBase(NativeMenu menu);
  virtual ~MenuBase();

  NativeMenu menu() const { return menu_; }

 private:
  friend class base::RefCounted<MenuBase>;

  void PlatformInit();

  NativeMenu menu_;
};

}  // namespace nu

#endif  // NATIVEUI_MENU_BASE_H_
