// Copyright 2020 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE.chromium file.

#include "nativeui/screen.h"

#include <utility>

#include "nativeui/state.h"

namespace nu {

bool Display::operator==(uint32_t display_id) const {
  return id == display_id;
}

bool Display::operator!=(const Display& other) const {
  return id != other.id ||
         scale_factor != other.scale_factor ||
#if defined(OS_MAC)
         internal != other.internal ||
#endif
         bounds != other.bounds ||
         work_area != other.work_area;
}

// static
Screen* Screen::GetCurrent() {
  return State::GetCurrent()->GetScreen();
}

Screen::Screen() : weak_factory_(this) {
  on_add_display.SetDelegate(this);
  on_remove_display.SetDelegate(this);
  on_update_display.SetDelegate(this);
}

Screen::~Screen() = default;

Display Screen::GetPrimaryDisplay() {
  NativeDisplay native = GetNativePrimaryDisplay();
  if (!native)
    return Display();
  return FindDisplay(native);
}

const Screen::DisplayList& Screen::GetAllDisplays() {
  if (!observer_) {
    observer_.reset(internal::ScreenObserver::Create(this));
    displays_ = CreateAllDisplays();
    // No more need to monitor signals after initialization.
    on_add_display.SetDelegate(nullptr);
    on_remove_display.SetDelegate(nullptr);
    on_update_display.SetDelegate(nullptr);
  }
  return displays_;
}

void Screen::NotifyDisplaysChange() {
  DisplayList old_displays = std::move(displays_);
  displays_ = CreateAllDisplays();

  // Display present in old_displays but not in new_displays has been removed.
  for (const auto& display : old_displays) {
    if (std::find(displays_.begin(), displays_.end(),
                  display.id) == displays_.end()) {
      on_remove_display.Emit(display);
    }
  }

  // Display present in new_displays but not in old_displays has been added.
  // Display present in both might have been modified.
  for (const auto& display : displays_) {
    auto old = std::find(old_displays.begin(), old_displays.end(), display.id);
    if (old == old_displays.end()) {
      on_add_display.Emit(display);
      continue;
    }

    if (display != *old)
      on_update_display.Emit(display);
  }
}

void Screen::OnConnect(int) {
  // Force initialization when user tries to listen to events.
  GetAllDisplays();
}

Display Screen::FindDisplay(NativeDisplay native) {
  const auto& all = GetAllDisplays();
  auto it = std::find(all.begin(), all.end(), DisplayIdFromNative(native));
  if (it == all.end())
    return CreateDisplayFromNative(native);
  return *it;
}

// static
Display Screen::CreatePrimaryDisplay() {
  NativeDisplay native = GetNativePrimaryDisplay();
  if (!native)
    return Display();
  return CreateDisplayFromNative(native);
}

}  // namespace nu
