// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/separator.h"

#include <gtk/gtk.h>

namespace nu {

NativeView Separator::PlatformCreate(Orientation orientation) {
  return gtk_separator_new(
      orientation == Orientation::Horizontal ? GTK_ORIENTATION_HORIZONTAL
                                             : GTK_ORIENTATION_VERTICAL);
}

}  // namespace nu
