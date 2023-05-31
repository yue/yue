// Copyright 2023 Cheng Zhao. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/global_shortcut.h"

#include <gdk/gdkx.h>

#include "nativeui/accelerator.h"
#include "nativeui/events/gtk/keyboard_code_conversion_gtk.h"

namespace nu {

namespace {

// The modifiers masks used for grabing keys. Due to XGrabKey only working on
// exact modifiers, we need to grab all key combination including zero or more
// of the following: Num lock, Caps lock and Scroll lock. So that we can make
// sure the behavior of global shortcuts is consistent on all platforms.
const unsigned int kModifiersMasks[] = {
  0,                                // No additional modifier.
  Mod2Mask,                         // Num lock
  LockMask,                         // Caps lock
  Mod5Mask,                         // Scroll lock
  Mod2Mask | LockMask,
  Mod2Mask | Mod5Mask,
  LockMask | Mod5Mask,
  Mod2Mask | LockMask | Mod5Mask
};

int GetNativeModifiers(const Accelerator& accelerator) {
  return (accelerator.IsShiftDown() ? ShiftMask : 0) |
         (accelerator.IsCtrlDown() ? ControlMask : 0) |
         (accelerator.IsAltDown() ? Mod1Mask : 0) |
         (accelerator.IsCmdDown() ? Mod4Mask : 0);
}

GdkFilterReturn RootWindowKeyFilter(XEvent* xevent, GdkEvent* event,
                                    GlobalShortcut* self) {
  if (xevent->type == KeyPress) {
    uint32_t keycode = xevent->xkey.keycode;
    int modifiers =
        xevent->xkey.state & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask);
    if (self->OnKeyPress({keycode, modifiers}))
      return GDK_FILTER_REMOVE;
  }
  return GDK_FILTER_CONTINUE;
}

}  // namespace

bool GlobalShortcut::OnKeyPress(NativeAccelerator na) {
  auto it = hot_key_ids_map_.find(na);
  if (it == hot_key_ids_map_.end())
    return false;
  for (int id : it->second)
    OnHotKeyEvent(id);
  return true;
}

void GlobalShortcut::StartWatching() {
  auto func = reinterpret_cast<GdkFilterFunc>(&RootWindowKeyFilter);
  gdk_window_add_filter(gdk_get_default_root_window(), func, this);
}

void GlobalShortcut::StopWatching() {
  auto func = reinterpret_cast<GdkFilterFunc>(&RootWindowKeyFilter);
  gdk_window_remove_filter(gdk_get_default_root_window(), func, this);
}

bool GlobalShortcut::PlatformRegister(const Accelerator& accelerator, int id) {
  int modifiers = GetNativeModifiers(accelerator);
  int keysym = XKeysymForWindowsKeyCode(accelerator.GetKeyCode(), false);
  GdkWindow* root = gdk_get_default_root_window();
  KeyCode keycode = ::XKeysymToKeycode(GDK_WINDOW_XDISPLAY(root), keysym);

  // Multiple registerations to the same accelerator.
  NativeAccelerator na = {keycode, modifiers};
  auto it = hot_key_ids_map_.find(na);
  if (it != hot_key_ids_map_.end()) {
    it->second.insert(id);
    return true;
  }

  GdkDisplay* display = gdk_display_get_default();
  gdk_x11_display_error_trap_push(display);

  // Because XGrabKey only works on the exact modifiers mask, we should register
  // our hot keys with modifiers that we want to ignore, including Num lock,
  // Caps lock, Scroll lock. See comment about |kModifiersMasks|.
  for (auto mask : kModifiersMasks) {
    ::XGrabKey(GDK_WINDOW_XDISPLAY(root), keycode, modifiers | mask,
               GDK_WINDOW_XID(root), False, GrabModeAsync, GrabModeAsync);
  }

  gdk_display_flush(display);
  if (gdk_x11_display_error_trap_pop(display)) {
    // We may have part of the hotkeys registered, clean up.
    for (auto mask : kModifiersMasks) {
      ::XUngrabKey(GDK_WINDOW_XDISPLAY(root), keycode, modifiers | mask,
                   GDK_WINDOW_XID(root));
    }
    return false;
  }

  hot_key_ids_map_[na] = {id};
  return true;
}

bool GlobalShortcut::PlatformUnregister(int id) {
  // Find the pair that includes the id.
  auto it = hot_key_ids_map_.begin();
  for (; it != hot_key_ids_map_.end(); ++it) {
    if (it->second.count(id) > 0)  // contains
      break;
  }
  if (it == hot_key_ids_map_.end())
    return false;

  // Do nothing if there are other registerations with same accelerator.
  if (it->second.size() > 1) {
    it->second.erase(id);
    return true;
  }

  GdkWindow* root = gdk_get_default_root_window();
  for (auto mask : kModifiersMasks) {
    ::XUngrabKey(GDK_WINDOW_XDISPLAY(root), it->first.first,
                 it->first.second | mask, GDK_WINDOW_XID(root));
  }

  hot_key_ids_map_.erase(it);
  return true;
}

}  // namespace nu
