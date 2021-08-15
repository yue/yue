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
  accelerators_[accelerator] = item->GetNative()->id;
  item->GetNative()->accelerator =
      base::UTF8ToWide(accelerator.GetShortcutText());
  // Refresh.
  item->SetLabel(item->GetNative()->label);
}

void AcceleratorManager::RemoveAccelerator(MenuItem* item,
                                           const Accelerator& accelerator) {
  accelerators_.erase(accelerator);
  item->GetNative()->accelerator.clear();
  // Refresh.
  item->SetLabel(item->GetNative()->label);
}

int AcceleratorManager::Process(const Accelerator& accelerator) {
  AcceleratorMap::iterator iter = accelerators_.find(accelerator);
  if (iter != accelerators_.end())
    return iter->second;
  return -1;
}

}  // namespace nu
