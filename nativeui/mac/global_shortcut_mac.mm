// Copyright 2023 Cheng Zhao. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/global_shortcut.h"

#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <IOKit/hidsystem/ev_keymap.h>

#include "base/apple/foundation_util.h"
#include "nativeui/accelerator.h"
#include "nativeui/events/mac/keyboard_code_conversion_mac.h"

namespace nu {

namespace {

OSStatus HotKeyHandler(EventHandlerCallRef next_handler,
                       EventRef event, void* user_data) {
  // Extract the hotkey from the event.
  EventHotKeyID hot_key_id;
  OSStatus result = ::GetEventParameter(event, kEventParamDirectObject,
      typeEventHotKeyID, nullptr, sizeof(hot_key_id), nullptr, &hot_key_id);
  if (result != noErr)
    return result;

  GlobalShortcut* global_shortcut = static_cast<GlobalShortcut*>(user_data);
  global_shortcut->OnHotKeyEvent(hot_key_id.id);
  return noErr;
}

}  // namespace

void GlobalShortcut::StartWatching() {
  if (event_handler_)
    return;
  EventHandlerUPP hot_key_function = ::NewEventHandlerUPP(&HotKeyHandler);
  EventTypeSpec event_type;
  event_type.eventClass = kEventClassKeyboard;
  event_type.eventKind = kEventHotKeyPressed;
  ::InstallApplicationEventHandler(
      hot_key_function, 1, &event_type, this, &event_handler_);
}

void GlobalShortcut::StopWatching() {
  if (!event_handler_)
    return;
  RemoveEventHandler(event_handler_);
  event_handler_ = nullptr;
}

bool GlobalShortcut::PlatformRegister(const Accelerator& accelerator, int id) {
  EventHotKeyID event_hot_key_id;

  // Signature uniquely identifies the application that owns this hot_key.
  event_hot_key_id.signature = base::apple::CreatorCodeForApplication();
  event_hot_key_id.id = id;

  // Translate ui::Accelerator modifiers to cmdKey, altKey, etc.
  int modifiers = 0;
  modifiers |= (accelerator.IsShiftDown() ? shiftKey : 0);
  modifiers |= (accelerator.IsCtrlDown() ? controlKey : 0);
  modifiers |= (accelerator.IsAltDown() ? optionKey : 0);
  modifiers |= (accelerator.IsCmdDown() ? cmdKey : 0);

  int key_code = MacKeyCodeForWindowsKeyCode(accelerator.GetKeyCode(), 0,
                                             nullptr, nullptr);

  // Register the event hot key.
  EventHotKeyRef hot_key_ref;
  OSStatus status = ::RegisterEventHotKey(key_code, modifiers, event_hot_key_id,
      ::GetApplicationEventTarget(), 0, &hot_key_ref);
  if (status != noErr)
    return false;

  id_hot_key_refs_[id] = hot_key_ref;
  return true;
}

bool GlobalShortcut::PlatformUnregister(int id) {
  auto it = id_hot_key_refs_.find(id);
  if (it == id_hot_key_refs_.end())
    return false;
  // Unregister the event hot key.
  EventHotKeyRef ref = it->second;
  ::UnregisterEventHotKey(ref);
  // Remove the event from the mapping.
  id_hot_key_refs_.erase(it);
  return true;
}

}  // namespace nu
