// Copyright 2020 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_GTK_GTK_THEME_H_
#define NATIVEUI_GFX_GTK_GTK_THEME_H_

#include <gtk/gtk.h>

#include <unordered_map>

#include "base/macros.h"
#include "nativeui/gfx/color.h"
#include "nativeui/signal.h"

namespace nu {

class GtkTheme {
 public:
  GtkTheme();
  ~GtkTheme();

  Color GetColor(Color::Name name);

  Signal<void()> on_theme_change;

 private:
  static void OnThemeChange(GtkSettings*, GParamSpec*, GtkTheme* self);

  // Cached colors.
  std::unordered_map<int, Color> colors_;

  gulong signal_gtk_theme_name_;
  gulong signal_gtk_prefer_dark_theme_;

  DISALLOW_COPY_AND_ASSIGN(GtkTheme);
};

}  // namespace nu

#endif  // NATIVEUI_GFX_GTK_GTK_THEME_H_
