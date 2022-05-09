// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/responder.h"

#include <gtk/gtk.h>

#include "nativeui/events/event.h"
#include "nativeui/window.h"

namespace nu {

namespace {

bool HandleViewDragging(GtkWidget* widget, GdkEvent* event, View* view) {
  // If user is dragging a widget that supports mouseDownMoveWindow, then we
  // need to move the window.
  if (event->motion.state & GDK_BUTTON1_MASK &&
      view->IsMouseDownCanMoveWindow()) {
    GtkWidget* toplevel = gtk_widget_get_toplevel(widget);
    if (gtk_widget_is_toplevel(toplevel)) {
      GdkWindow* window = gtk_widget_get_window(toplevel);
      gdk_window_begin_move_drag(window, 1,
                                 event->motion.x_root, event->motion.y_root,
                                 event->motion.time);
      return true;
    }
  }
  return false;
}

gboolean OnMouseMove(GtkWidget* widget, GdkEvent* event, Responder* responder) {
  if (responder->GetClassName() != Window::kClassName &&
      HandleViewDragging(widget, event, static_cast<View*>(responder)))
    return true;
  if (!responder->on_mouse_move.IsEmpty()) {
    responder->on_mouse_move.Emit(responder, MouseEvent(event, widget));
    return false;
  }
  return false;
}

gboolean OnMouseEvent(GtkWidget* widget, GdkEvent* event,
                      Responder* responder) {
  switch (event->any.type) {
    case GDK_BUTTON_PRESS:
      return responder->on_mouse_down.Emit(responder,
                                           MouseEvent(event, widget));
    case GDK_BUTTON_RELEASE:
      return responder->on_mouse_up.Emit(responder, MouseEvent(event, widget));
    case GDK_ENTER_NOTIFY:
      responder->on_mouse_enter.Emit(responder, MouseEvent(event, widget));
      return false;
    case GDK_LEAVE_NOTIFY:
      responder->on_mouse_leave.Emit(responder, MouseEvent(event, widget));
      return false;
    default:
      return false;
  }
}

gboolean OnKeyDown(GtkWidget* widget, GdkEvent* event, Responder* responder) {
  return responder->on_key_down.Emit(responder, KeyEvent(event, widget));
}

gboolean OnKeyUp(GtkWidget* widget, GdkEvent* event, Responder* responder) {
  return responder->on_key_up.Emit(responder, KeyEvent(event, widget));
}

}  // namespace

void Responder::PlatformInstallMouseClickEvents() {
  g_signal_connect(GetNative(), "button-press-event",
                   G_CALLBACK(OnMouseEvent), this);
  g_signal_connect(GetNative(), "button-release-event",
                   G_CALLBACK(OnMouseEvent), this);
}

void Responder::PlatformInstallMouseMoveEvents() {
  g_signal_connect(GetNative(), "motion-notify-event",
                   G_CALLBACK(OnMouseMove), this);
  g_signal_connect(GetNative(), "enter-notify-event",
                   G_CALLBACK(OnMouseEvent), this);
  g_signal_connect(GetNative(), "leave-notify-event",
                   G_CALLBACK(OnMouseEvent), this);
}

void Responder::PlatformInstallKeyEvents() {
  g_signal_connect(GetNative(), "key-press-event",
                   G_CALLBACK(OnKeyDown), this);
  g_signal_connect(GetNative(), "key-release-event",
                   G_CALLBACK(OnKeyUp), this);
}

}  // namespace nu
