// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

namespace nu {

void Container::PlatformInit() {
  set_view(gtk_fixed_new());
  g_object_ref_sink(view());
}

}  // namespace nu

