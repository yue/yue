// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/lifetime.h"

#include <gtk/gtk.h>

namespace nu {

void Lifetime::PlatformInit() {
  gtk_init(nullptr, nullptr);
}

void Lifetime::PlatformDestroy() {
}

}  // namespace nu
