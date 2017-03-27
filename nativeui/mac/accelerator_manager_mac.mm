// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/accelerator_manager.h"

#import <Cocoa/Cocoa.h>

#include "nativeui/events/keyboard_code_conversion_mac.h"
#include "nativeui/menu_item.h"

namespace nu {

AcceleratorManager::AcceleratorManager() {
}

AcceleratorManager::~AcceleratorManager() {
}

void AcceleratorManager::RegisterAccelerator(MenuItem* item,
                                             const Accelerator& accelerator) {
  unichar character;
  unichar characterIgnoringModifiers;
  MacKeyCodeForWindowsKeyCode(accelerator.GetKeyCode(),
                              accelerator.GetModifiers(),
                              &character,
                              &characterIgnoringModifiers);
  NSMenuItem* menu_item = item->menu_item();
  menu_item.keyEquivalentModifierMask = accelerator.GetModifiers();
  menu_item.keyEquivalent =
      [[[NSString alloc] initWithCharacters:&character length:1] autorelease];
}

void AcceleratorManager::RemoveAccelerator(MenuItem* item,
                                           const Accelerator& accelerator) {
  NSMenuItem* menu_item = item->menu_item();
  menu_item.keyEquivalent = @"";
  menu_item.keyEquivalentModifierMask = 0;
}

}  // namespace nu
