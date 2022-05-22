// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/responder.h"

#include <gtk/gtk.h>

#include "nativeui/events/event.h"
#include "nativeui/gtk/nu_container.h"
#include "nativeui/window.h"

namespace nu {

namespace {

// The view that has the capture.
Responder* g_grabbed_responder = nullptr;

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
  if (responder->GetType() == Responder::Type::View &&
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

void Responder::SetCapture() {
  // Get the GDK window.
  GdkWindow* window;
  if (NU_IS_CONTAINER(GetNative()))
    window = nu_container_get_window(NU_CONTAINER(GetNative()));
  else
    window = gtk_widget_get_window(GetNative());
  if (!window)
    return;

  const GdkEventMask mask = GdkEventMask(GDK_BUTTON_PRESS_MASK |
                                         GDK_BUTTON_RELEASE_MASK |
                                         GDK_POINTER_MOTION_HINT_MASK |
                                         GDK_POINTER_MOTION_MASK);
  if (gdk_pointer_grab(window, FALSE, mask, NULL, NULL,
                       GDK_CURRENT_TIME) == GDK_GRAB_SUCCESS)
    g_grabbed_responder = this;
}

void Responder::ReleaseCapture() {
  gdk_pointer_ungrab(GDK_CURRENT_TIME);

  // In X11 the grab can not be hijacked by other applications, so the only
  // possible case for losing capture is to call this function.
  if (g_grabbed_responder) {
    g_grabbed_responder->on_capture_lost.Emit(g_grabbed_responder);
    g_grabbed_responder = nullptr;
  }
}

bool Responder::HasCapture() const {
  return gdk_pointer_is_grabbed() && g_grabbed_responder == this;
}

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
