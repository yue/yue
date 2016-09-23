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

class MenuItem;

NATIVEUI_EXPORT class MenuBase : public base::RefCounted<MenuBase> {
 public:
  void Append(MenuItem* item);
  void Insert(MenuItem* item, int index);
  void Remove(MenuItem* item);

  int item_count() const { return static_cast<int>(items_.size()); }
  MenuItem* item_at(int index) const {
    if (index < 0 || index >= item_count())
      return nullptr;
    return items_[index].get();
  }

  NativeMenu menu() const { return menu_; }

 protected:
  explicit MenuBase(NativeMenu menu);
  virtual ~MenuBase();

 private:
  friend class base::RefCounted<MenuBase>;

  void PlatformInit();
  void PlatformDestroy();
  void PlatformInsert(MenuItem* item, int index);
  void PlatformRemove(MenuItem* item);

  NativeMenu menu_;

  // Relationships.
  MenuBase* parent_ = nullptr;
  std::vector<scoped_refptr<MenuItem>> items_;
};

}  // namespace nu

#endif  // NATIVEUI_MENU_BASE_H_
