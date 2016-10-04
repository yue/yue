// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/accelerator_manager.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/menu_item.h"
#include "nativeui/win/menu_item_win.h"

namespace nu {

AcceleratorManager::AcceleratorManager() {
}

AcceleratorManager::~AcceleratorManager() {
}

void AcceleratorManager::RegisterAccelerator(MenuItem* item,
                                             const Accelerator& accelerator) {
  accelerators_[accelerator] = item->menu_item()->id;
  item->menu_item()->accelerator =
      base::ASCIIToUTF16(accelerator.GetShortcutText());
  // Refresh.
  item->SetLabel(item->menu_item()->label);
}

void AcceleratorManager::RemoveAccelerator(MenuItem* item,
                                           const Accelerator& accelerator) {
  accelerators_.erase(accelerator);
  item->menu_item()->accelerator.clear();
  // Refresh.
  item->SetLabel(item->menu_item()->label);
}

int AcceleratorManager::Process(const Accelerator& accelerator) {
  AcceleratorMap::iterator iter = accelerators_.find(accelerator);
  if (iter != accelerators_.end())
    return iter->second;
  return -1;
}

}  // namespace nu
