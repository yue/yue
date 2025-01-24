// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/appearance.h"

#include "base/time/time.h"
#include "base/win/dark_mode_support.h"
#include "base/win/registry.h"
#include "nativeui/win/util/win32_window.h"

namespace nu {

namespace internal {

class ColorSchemeObserverImpl : public ColorSchemeObserver,
                                public Win32Window {
 public:
  explicit ColorSchemeObserverImpl(Appearance* appearance)
      : appearance_(appearance) {}

 private:
  // Win32Window:
  bool ProcessWindowMessage(HWND window,
                            UINT message,
                            WPARAM w_param,
                            LPARAM l_param,
                            LRESULT* result) override {
    if (message == WM_SETTINGCHANGE &&
        CompareStringOrdinal(reinterpret_cast<LPCWCH>(l_param), -1,
                             L"ImmersiveColorSet", -1, TRUE) == CSTR_EQUAL) {
      // The ImmersiveColorSet is emitted multiple times for one change, try to
      // reduce the event numbers to minimum.
      base::Time now = base::Time::Now();
      if (now - last_event > base::Milliseconds(500)) {
        appearance_->on_color_scheme_change.Emit();
        last_event = now;
      }
    }
    return false;
  }

  Appearance* appearance_;
  base::Time last_event;
};

// static
ColorSchemeObserver* ColorSchemeObserver::Create(Appearance* appearance) {
  return new ColorSchemeObserverImpl(appearance);
}

}  // namespace internal

void Appearance::SetDarkModeEnabled(bool enable) {
  if (!base::win::IsDarkModeAvailable())
    return;
  dark_mode_enabled_ = enable;
  base::win::AllowDarkModeForApp(enable);
}

bool Appearance::IsDarkScheme() const {
  if (!dark_mode_enabled_)
    return false;
  base::win::RegKey hkcu_themes_regkey;
  if (ERROR_SUCCESS != hkcu_themes_regkey.Open(
          HKEY_CURRENT_USER,
          L"Software\\Microsoft\\Windows\\CurrentVersion\\"
          L"Themes\\Personalize",
          KEY_READ)) {
    return false;
  }
  DWORD apps_use_light_theme = 1;
  hkcu_themes_regkey.ReadValueDW(L"AppsUseLightTheme",
                                 &apps_use_light_theme);
  return apps_use_light_theme == 0;
}

}  // namespace nu
