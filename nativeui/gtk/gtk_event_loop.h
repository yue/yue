// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_GTK_EVENT_LOOP_H_
#define NATIVEUI_GTK_GTK_EVENT_LOOP_H_

#include "base/macros.h"
#include "nativeui/types.h"

namespace nu {

// The message loop of base only iterates through glib events, this class
// hooks to gdk events and redirect them to GTK.
class GtkEventLoop {
 public:
  GtkEventLoop();
  ~GtkEventLoop();

 private:
  static void DispatchGdkEvent(GdkEvent* gdk_event, gpointer);

  DISALLOW_COPY_AND_ASSIGN(GtkEventLoop);
};

}  // namespace nu

#endif  // NATIVEUI_GTK_GTK_EVENT_LOOP_H_
