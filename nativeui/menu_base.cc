// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_base.h"

#include <algorithm>

#include "nativeui/menu_item.h"

namespace nu {

MenuBase::MenuBase(NativeMenu menu) : menu_(menu) {
  PlatformInit();
}

MenuBase::~MenuBase() {
  PlatformDestroy();
}

void MenuBase::Append(MenuItem* item) {
  Insert(item, item_count());
}

void MenuBase::Insert(MenuItem* item, int index) {
  if (!item || item->menu() || index < 0 || index > item_count())
    return;
  items_.insert(items_.begin() + index, item);
  item->set_menu(this);
  PlatformInsert(item, index);
  item->SetAcceleratorManager(accel_manager_);
}

void MenuBase::Remove(MenuItem* item) {
  const auto i(std::find(items_.begin(), items_.end(), item));
  if (i == items_.end())
    return;
  item->SetAcceleratorManager(nullptr);
  PlatformRemove(item);
  item->set_menu(nullptr);
  items_.erase(i);
}

void MenuBase::SetAcceleratorManager(AcceleratorManager* accel_manager) {
  accel_manager_ = accel_manager;
  for (int i = 0; i < item_count(); ++i)
    item_at(i)->SetAcceleratorManager(accel_manager);
}

}  // namespace nu
