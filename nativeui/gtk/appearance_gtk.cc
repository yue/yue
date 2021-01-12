// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/appearance.h"

#include "nativeui/gfx/color.h"
#include "nativeui/gfx/gtk/gtk_theme.h"
#include "nativeui/state.h"

namespace nu {

namespace internal {

class ColorSchemeObserverImpl : public ColorSchemeObserver {
 public:
  explicit ColorSchemeObserverImpl(Appearance* appearance) {
    auto* gtk_theme = State::GetCurrent()->GetGtkTheme();
    signal_ = gtk_theme->on_theme_change.Connect([appearance]() {
      appearance->on_color_scheme_change.Emit();
    });
  }

  ~ColorSchemeObserverImpl() override {
    auto* gtk_theme = State::GetCurrent()->GetGtkTheme();
    gtk_theme->on_theme_change.Disconnect(signal_);
  }

 private:
  int signal_;
};

// static
ColorSchemeObserver* ColorSchemeObserver::Create(Appearance* appearance) {
  return new ColorSchemeObserverImpl(appearance);
}

}  // namespace internal

bool Appearance::IsDarkScheme() const {
  Color fg = Color::Get(Color::Name::Text);
  Color bg = Color::Get(Color::Name::Control);
  return fg.GetRelativeLuminance() > bg.GetRelativeLuminance();
}

}  // namespace nu
