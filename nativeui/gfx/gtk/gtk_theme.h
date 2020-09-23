// Copyright 2020 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_GTK_GTK_THEME_H_
#define NATIVEUI_GFX_GTK_GTK_THEME_H_

#include <unordered_map>

#include "base/macros.h"
#include "nativeui/gfx/color.h"

namespace nu {

class GtkTheme {
 public:
  GtkTheme();
  ~GtkTheme();

  Color GetColor(Color::Name name);

 private:
  // Cached colors.
  std::unordered_map<int, Color> colors_;

  DISALLOW_COPY_AND_ASSIGN(GtkTheme);
};

}  // namespace nu

#endif  // NATIVEUI_GFX_GTK_GTK_THEME_H_
