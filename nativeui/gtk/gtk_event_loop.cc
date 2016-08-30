// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/gtk_event_loop.h"

#include <gdk/gdk.h>

namespace nu {

GtkEventLoop::GtkEventLoop() {
  gdk_event_handler_set(DispatchGdkEvent, NULL, NULL);
}

GtkEventLoop::~GtkEventLoop() {
  gdk_event_handler_set(reinterpret_cast<GdkEventFunc>(gtk_main_do_event),
                        NULL, NULL);
}

// static
void GtkEventLoop::DispatchGdkEvent(GdkEvent* gdk_event, gpointer) {
  gtk_main_do_event(gdk_event);
}

}  // namespace nu
