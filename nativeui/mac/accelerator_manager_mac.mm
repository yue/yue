// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/accelerator_manager.h"

#import <Cocoa/Cocoa.h>

#include "nativeui/events/mac/keyboard_code_conversion_mac.h"
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

  // Make some special cases more visual friendly.
  int modifiers = accelerator.GetModifiers();
  if (character != characterIgnoringModifiers)
    modifiers ^= NSShiftKeyMask;
  if (character == NSDeleteFunctionKey)
    character = NSDeleteCharacter;

  NSMenuItem* menu_item = item->GetNative();
  menu_item.keyEquivalentModifierMask = modifiers;
  menu_item.keyEquivalent =
      [[[NSString alloc] initWithCharacters:&character length:1] autorelease];
}

void AcceleratorManager::RemoveAccelerator(MenuItem* item,
                                           const Accelerator& accelerator) {
  NSMenuItem* menu_item = item->GetNative();
  menu_item.keyEquivalent = @"";
  menu_item.keyEquivalentModifierMask = 0;
}

}  // namespace nu
