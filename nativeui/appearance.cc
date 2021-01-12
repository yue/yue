// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/appearance.h"

#include "nativeui/state.h"

namespace nu {

// static
Appearance* Appearance::GetCurrent() {
  return State::GetCurrent()->GetAppearance();
}

Appearance::Appearance() : weak_factory_(this) {
  on_color_scheme_change.SetDelegate(this, kColorSchemeChange);
}

Appearance::~Appearance() = default;

void Appearance::OnConnect(int identifier) {
  if (identifier == kColorSchemeChange) {
    if (!color_scheme_observer_)
      color_scheme_observer_.reset(internal::ColorSchemeObserver::Create(this));
  }
}

}  // namespace nu
