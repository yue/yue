// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MENU_ITEM_H_
#define NATIVEUI_MENU_ITEM_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "nativeui/signal.h"
#include "nativeui/types.h"

namespace nu {

class MenuBase;

NATIVEUI_EXPORT class MenuItem : public base::RefCounted<MenuItem> {
 public:
  enum Type {
    Label,
    CheckBox,
    Radio,
    Separator,
    Submenu,
  };

  explicit MenuItem(Type type);

  void SetLabel(const std::string& label);
  std::string GetLabel() const;

  void SetSubmenu(MenuBase* submenu);
  MenuBase* GetSubmenu() const;

  // Events.
  Signal<void()> on_click;

  // Only used internally to set the owner of menu item.
  void set_menu(MenuBase* menu) { menu_ = menu; }
  MenuBase* menu() const { return menu_; }

  // Return the native MenuItem object.
  NativeMenuItem menu_item() const { return menu_item_; }

 private:
  friend class base::RefCounted<MenuItem>;

  ~MenuItem();

  void PlatformInit();
  void PlatformDestroy();
  void PlatformSetSubmenu(MenuBase* submenu);

  // Weak ref to the owner menu.
  MenuBase* menu_ = nullptr;

  // The submenu.
  scoped_refptr<MenuBase> submenu_;

  Type type_;
  NativeMenuItem menu_item_;
};

}  // namespace nu

#endif  // NATIVEUI_MENU_ITEM_H_
