// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/global_shortcut.h"

#include "nativeui/accelerator.h"
#include "nativeui/win/util/win32_window.h"

namespace nu {

class GlobalShortcutHost : public Win32Window {
 public:
  explicit GlobalShortcutHost(GlobalShortcut* shortcut): shortcut_(shortcut) {}

 protected:
  bool ProcessWindowMessage(HWND window,
                            UINT message,
                            WPARAM w_param,
                            LPARAM l_param,
                            LRESULT* result) override {
    if (message == WM_HOTKEY)
      shortcut_->OnHotKeyEvent(static_cast<int>(w_param));
    return false;
  }

 private:
  GlobalShortcut* shortcut_;
};

void GlobalShortcut::StartWatching() {
  if (host_)
    return;
  host_ = new GlobalShortcutHost(this);
}

void GlobalShortcut::StopWatching() {
  delete host_;
  host_ = nullptr;
}

bool GlobalShortcut::PlatformRegister(const Accelerator& accelerator, int id) {
  // Convert Accelerator modifiers to OS modifiers.
  int modifiers = MOD_NOREPEAT;
  modifiers |= accelerator.IsShiftDown() ? MOD_SHIFT : 0;
  modifiers |= accelerator.IsCtrlDown() ? MOD_CONTROL : 0;
  modifiers |= accelerator.IsAltDown() ? MOD_ALT : 0;
  modifiers |= accelerator.IsCmdDown() ? MOD_WIN : 0;

  return ::RegisterHotKey(host_->hwnd(), id, modifiers,
                          accelerator.GetKeyCode());
}

bool GlobalShortcut::PlatformUnregister(int id) {
  return ::UnregisterHotKey(host_->hwnd(), id);
}

}  // namespace nu
