// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/state.h"

#include "nativeui/gfx/gtk/gtk_theme.h"

namespace nu {

void State::PlatformInit() {
}

GtkTheme* State::GetGtkTheme() {
  if (!gtk_theme_)
    gtk_theme_.reset(new GtkTheme);
  return gtk_theme_.get();
}

}  // namespace nu
