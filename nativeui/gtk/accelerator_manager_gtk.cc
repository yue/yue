// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/accelerator_manager.h"

#if defined(OS_LINUX)
#include <gtk/gtk.h>
#endif

#include "nativeui/menu_item.h"

namespace nu {

AcceleratorManager::AcceleratorManager()
    : accel_group_(gtk_accel_group_new()) {
  g_object_ref_sink(accel_group_);
}

AcceleratorManager::~AcceleratorManager() {
  g_object_unref(accel_group_);
}

void AcceleratorManager::RegisterAccelerator(MenuItem* item,
                                             const Accelerator& accelerator) {
  gtk_widget_add_accelerator(
      GTK_WIDGET(item->GetNative()), "activate", accel_group_,
      accelerator.GetKeyCode(),
      static_cast<GdkModifierType>(accelerator.GetModifiers()),
      GTK_ACCEL_VISIBLE);
}

void AcceleratorManager::RemoveAccelerator(MenuItem* item,
                                           const Accelerator& accelerator) {
  gtk_widget_remove_accelerator(
      GTK_WIDGET(item->GetNative()), accel_group_,
      accelerator.GetKeyCode(),
      static_cast<GdkModifierType>(accelerator.GetModifiers()));
}

}  // namespace nu
