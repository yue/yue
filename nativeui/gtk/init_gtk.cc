// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/init.h"

#include "nativeui/gtk/gtk_event_loop.h"

namespace nu {

void Initialize() {
  gtk_init(nullptr, nullptr);
  GtkEventLoop::GetInstance();
}

}  // namespace nu

