// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MENU_ITEM_H_
#define NATIVEUI_MENU_ITEM_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "nativeui/accelerator.h"
#include "nativeui/signal.h"
#include "nativeui/types.h"

namespace nu {

class AcceleratorManager;
class Menu;
class MenuBase;

class NATIVEUI_EXPORT MenuItem : public base::RefCounted<MenuItem> {
 public:
  enum class Type {
    Label,
    Checkbox,
    Radio,
    Separator,
    Submenu,
  };

  enum class Role {
    // Item roles.
    // menu_item_gtk.cc should be updated when new roles are added.
    Copy,
    Cut,
    Paste,
    SelectAll,
    Undo,
    Redo,
    // Indicate number of item roles, should not be used.
    ItemCount,
    // Submenu roles.
    Help,
    Window,
    Services,
    // Indicate a normal item, should not be used.
    None,
  };

  explicit MenuItem(Type type);
  explicit MenuItem(Role role);

  void Click();

  void SetLabel(const std::string& label);
  std::string GetLabel() const;

  void SetSubmenu(Menu* submenu);
  Menu* GetSubmenu() const;

  void SetChecked(bool checked);
  bool IsChecked() const;

  void SetEnabled(bool enabled);
  bool IsEnabled() const;

  void SetVisible(bool visible);
  bool IsVisible() const;

  void SetAccelerator(const Accelerator& accelerator);
  Accelerator GetAccelerator() const;

  // Return the type of menu item.
  Type GetType() const { return type_; }

  // Return the role of menu item.
  Role GetRole() const { return role_; }

  // Get the owner of this item.
  MenuBase* GetMenu() const { return menu_; }

  // Return the native MenuItem object.
  NativeMenuItem GetNative() const { return menu_item_; }

  // Events.
  Signal<void(MenuItem*)> on_click;

  // Internal: Set the owner of menu item.
  void set_menu(MenuBase* menu) { menu_ = menu; }

  // Internal: Search for the top-level menu.
  MenuBase* FindTopLevelMenu() const;

 private:
  friend class MenuBase;
  friend class base::RefCounted<MenuItem>;

  ~MenuItem();

  void PlatformInit();
  void PlatformDestroy();
  void PlatformSetSubmenu(Menu* submenu);
  void PlatformSetAccelerator(const Accelerator& accelerator);
  void PlatformRemoveAccelerator();

  // Flip all radio items in the same group with |item|.
  void FlipRadioMenuItems(nu::MenuBase* menu, nu::MenuItem* sender);

  // Called by menu to change the AcceleratorManager.
  void SetAcceleratorManager(AcceleratorManager* accel_manager);

  // Weak ref to the owner menu.
  MenuBase* menu_ = nullptr;

  // Role of the menu.
  Role role_ = Role::None;

  // The submenu.
  scoped_refptr<Menu> submenu_;

  // Stored accelerator instance.
  Accelerator accelerator_;

  // Weak ref to the AcceleratorManager.
  AcceleratorManager* accel_manager_ = nullptr;

  Type type_;
  NativeMenuItem menu_item_;
};

}  // namespace nu

#endif  // NATIVEUI_MENU_ITEM_H_
