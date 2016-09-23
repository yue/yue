// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MENU_ITEM_H_
#define NATIVEUI_MENU_ITEM_H_

#include "base/memory/ref_counted.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"

namespace nu {

class MenuItem : public base::RefCounted<MenuItem> {
 public:
  enum Type {
    Label,
    CheckBox,
    Radio,
    Separator,
  };

  explicit MenuItem(Type type);

  NativeMenuItem menu_item() const { return menu_item_; }

 private:
  friend class base::RefCounted<MenuItem>;

  ~MenuItem();

  void PlatformInit();
  void PlatformDestroy();

  Type type_;
  NativeMenuItem menu_item_;
};

}  // namespace nu

#endif  // NATIVEUI_MENU_ITEM_H_
