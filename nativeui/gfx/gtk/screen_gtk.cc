// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/screen.h"

#include <gtk/gtk.h>

#include "nativeui/slider.h"

namespace nu {

float GetScaleFactor() {
  static float scale_factor = -1.f;
  if (scale_factor <= 0) {
    // The gtk-xft-dpi GtkSetting does not return us correct value, the only
    // reliable way to get scale factor on Linux seems to be calling the
    // gtk_widget_get_scale_factor API.
    scoped_refptr<nu::Slider> widget = new nu::Slider;
    scale_factor = gtk_widget_get_scale_factor(widget->GetNative());
  }
  return scale_factor;
}

}  // namespace nu
