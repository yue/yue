// Copyright 2016 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/color.h"

#include <gdk/gdk.h>

#include "nativeui/gfx/gtk/gtk_theme.h"
#include "nativeui/state.h"

namespace nu {

// static
Color Color::Get(Name name) {
  return State::GetCurrent()->GetGtkTheme()->GetColor(name);
}

GdkRGBA Color::ToGdkRGBA() const {
  GdkRGBA rgba = { r() / 255., g() / 255., b() / 255., a() / 255. };
  return rgba;
}

}  // namespace nu
