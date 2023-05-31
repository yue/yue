// Copyright 2023 Cheng Zhao. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_GLOBAL_SHORTCUT_H_
#define NATIVEUI_GLOBAL_SHORTCUT_H_

#include <functional>
#include <map>
#include <set>
#include <utility>

#include "base/memory/weak_ptr.h"
#include "nativeui/nativeui_export.h"

#if defined(OS_MAC)
typedef struct OpaqueEventHotKeyRef* EventHotKeyRef;
typedef struct OpaqueEventHandlerRef* EventHandlerRef;
#endif

namespace nu {

class Accelerator;
#if defined(OS_WIN)
class GlobalShortcutHost;
#endif

class NATIVEUI_EXPORT GlobalShortcut {
 public:
  static GlobalShortcut* GetCurrent();

  ~GlobalShortcut();

  GlobalShortcut(const GlobalShortcut&) = delete;
  GlobalShortcut& operator=(const GlobalShortcut&) = delete;

  int Register(const Accelerator& accelerator, std::function<void()> callback);
  void Unregister(int id);
  void UnregisterAll();

  // Internal: Keyboard event callbacks.
  void OnHotKeyEvent(int id);

#if defined(OS_LINUX)
  using NativeAccelerator = std::pair<uint32_t, int>;

  // Internal: Notify native keypress event.
  bool OnKeyPress(NativeAccelerator na);
#endif

  base::WeakPtr<GlobalShortcut> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

 protected:
  GlobalShortcut();

 private:
  friend class State;

  void StartWatching();
  void StopWatching();
  bool PlatformRegister(const Accelerator& accelerator, int id);
  bool PlatformUnregister(int id);

  int next_id_ = 1;
  std::map<int, std::function<void()>> callbacks_;
  std::map<int, Accelerator> accelerators_;

#if defined(OS_LINUX)
  // A native accelerator map to multiple IDs.
  std::map<NativeAccelerator, std::set<int>> hot_key_ids_map_;
#endif

#if defined(OS_MAC)
  // Keyboard shortcut IDs to hotkeys map for unregistration.
  std::map<int, EventHotKeyRef> id_hot_key_refs_;
  // Event handler for keyboard shortcut hot keys.
  EventHandlerRef event_handler_ = nullptr;
#endif

#if defined(OS_WIN)
  GlobalShortcutHost* host_ = nullptr;
#endif

  base::WeakPtrFactory<GlobalShortcut> weak_factory_;
};

}  // namespace nu

#endif  // NATIVEUI_GLOBAL_SHORTCUT_H_
