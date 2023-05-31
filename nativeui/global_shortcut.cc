// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/global_shortcut.h"

#include <utility>

#include "nativeui/accelerator.h"
#include "nativeui/state.h"

namespace nu {

// static
GlobalShortcut* GlobalShortcut::GetCurrent() {
  return State::GetCurrent()->GetGlobalShortcut();
}

GlobalShortcut::GlobalShortcut() : weak_factory_(this) {}

GlobalShortcut::~GlobalShortcut() {
  if (!callbacks_.empty())
    StopWatching();
}

int GlobalShortcut::Register(const Accelerator& accelerator,
                             std::function<void()> callback) {
  if (callbacks_.empty())
    StartWatching();
  if (!PlatformRegister(accelerator, next_id_))
    return -1;
  callbacks_.emplace(next_id_, std::move(callback));
  accelerators_.emplace(next_id_, accelerator);
  return next_id_++;
}

void GlobalShortcut::Unregister(int id) {
  if (!PlatformUnregister(id))
    return;
  callbacks_.erase(id);
  accelerators_.erase(id);
  if (callbacks_.empty())
    StopWatching();
}

void GlobalShortcut::UnregisterAll() {
  for (const auto& item : callbacks_)
    PlatformUnregister(item.first);
  callbacks_.clear();
  accelerators_.clear();
  StopWatching();
}

void GlobalShortcut::OnHotKeyEvent(int id) {
  auto it = callbacks_.find(id);
  if (it != callbacks_.end())
    it->second();
}

}  // namespace nu
