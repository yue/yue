// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MENU_BASE_H_
#define NATIVEUI_MENU_BASE_H_

#include <vector>

#include "base/memory/ref_counted.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"

namespace nu {

class AcceleratorManager;
class MenuItem;

class NATIVEUI_EXPORT MenuBase : public base::RefCounted<MenuBase> {
 public:
  // Return the receiving view's class name. A view class is a string which
  // uniquely identifies the view class. It is intended to be used as a way to
  // find out during run time if a view can be safely casted to a specific view
  // subclass.
  virtual const char* GetClassName() const = 0;

  void Append(scoped_refptr<MenuItem> item);
  void Insert(scoped_refptr<MenuItem> item, int index);
  void Remove(MenuItem* item);

  int ItemCount() const { return static_cast<int>(items_.size()); }
  MenuItem* ItemAt(int index) const {
    if (index < 0 || index >= ItemCount())
      return nullptr;
    return items_[index].get();
  }

  // Return the native Menu object.
  NativeMenu GetNative() const { return menu_; }

  // Internal: Relationships with submenu items.
  void SetParent(MenuItem* item) { parent_ = item; }
  MenuItem* GetParent() const { return parent_; }

  // Internal: Notify the change of AcceleratorManager.
  void SetAcceleratorManager(AcceleratorManager* accel_manager);

#if defined(OS_LINUX) || defined(OS_WIN)
  // Internal: Update items on menu show/hide.
  void OnMenuShow();
  void OnMenuHide();
#endif

 protected:
  explicit MenuBase(NativeMenu menu);
  virtual ~MenuBase();

 private:
  friend class base::RefCounted<MenuBase>;

  void PlatformInit();
  void PlatformDestroy();
  void PlatformInsert(MenuItem* item, int index);
  void PlatformRemove(MenuItem* item);

  // Weak ref to the AcceleratorManager.
  AcceleratorManager* accel_manager_ = nullptr;

  // Relationships.
  MenuItem* parent_ = nullptr;
  std::vector<scoped_refptr<MenuItem>> items_;

  NativeMenu menu_;
};

}  // namespace nu

#endif  // NATIVEUI_MENU_BASE_H_
