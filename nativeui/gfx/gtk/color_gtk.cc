// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/color.h"

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "nativeui/label.h"

namespace nu {

GdkRGBA Color::ToGdkRGBA() const {
  GdkRGBA rgba = { r() / 255., g() / 255., b() / 255., a() / 255. };
  return rgba;
}

Color GetThemeColor(Color::Theme theme) {
  if (theme == Color::Theme::Text) {
    scoped_refptr<nu::Label> label = new nu::Label;
    gtk_widget_ensure_style(label->GetNative());
    GtkStyle* style = gtk_widget_get_style(label->GetNative());
    GdkColor color;
    if (!gtk_style_lookup_color(style, "text_color", &color))
      return Color();
    return Color(color.red >> 8, color.green >> 8, color.blue >> 8);
  } else {
    return Color();
  }
}

}  // namespace nu
