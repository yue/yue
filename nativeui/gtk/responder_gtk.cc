// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/responder.h"

#include <gtk/gtk.h>

#include "nativeui/events/event.h"
#include "nativeui/window.h"

namespace nu {

namespace {

template<typename T>
bool HandleViewDragging(GtkWidget*, GdkEvent*, T*) {
  return false;
}

template<>
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

template<typename T>
gboolean OnMouseMove(GtkWidget* widget, GdkEvent* event, T* view) {
  if (HandleViewDragging(widget, event, view))
    return true;
  if (!view->on_mouse_move.IsEmpty()) {
    view->on_mouse_move.Emit(view, MouseEvent(event, widget));
    return false;
  }
  return false;
}

template<typename T>
gboolean OnMouseEvent(GtkWidget* widget, GdkEvent* event, T* view) {
  switch (event->any.type) {
    case GDK_BUTTON_PRESS:
      return view->on_mouse_down.Emit(view, MouseEvent(event, widget));
    case GDK_BUTTON_RELEASE:
      return view->on_mouse_up.Emit(view, MouseEvent(event, widget));
    case GDK_ENTER_NOTIFY:
      view->on_mouse_enter.Emit(view, MouseEvent(event, widget));
      return false;
    case GDK_LEAVE_NOTIFY:
      view->on_mouse_leave.Emit(view, MouseEvent(event, widget));
      return false;
    default:
      return false;
  }
}

template<typename T>
gboolean OnKeyDown(GtkWidget* widget, GdkEvent* event, T* view) {
  return view->on_key_down.Emit(view, KeyEvent(event, widget));
}

template<typename T>
gboolean OnKeyUp(GtkWidget* widget, GdkEvent* event, T* view) {
  return view->on_key_up.Emit(view, KeyEvent(event, widget));
}

}  // namespace

template<typename T>
Responder<T>::Responder() {
  // Lazy install event handlers.
  on_mouse_down.SetDelegate(this, kOnMouseClick);
  on_mouse_up.SetDelegate(this, kOnMouseClick);
  on_mouse_move.SetDelegate(this, kOnMouseMove);
  on_mouse_enter.SetDelegate(this, kOnMouseMove);
  on_mouse_leave.SetDelegate(this, kOnMouseMove);
  on_key_down.SetDelegate(this, kOnKey);
  on_key_up.SetDelegate(this, kOnKey);
}

template<typename T>
void Responder<T>::OnConnect(int identifier) {
  auto native = static_cast<T*>(this)->GetNative();
  switch (identifier) {
    case kOnMouseClick:
      if (!on_mouse_click_installed_) {
        g_signal_connect(native, "button-press-event",
                         G_CALLBACK(OnMouseEvent<T>), this);
        g_signal_connect(native, "button-release-event",
                         G_CALLBACK(OnMouseEvent<T>), this);
        on_mouse_click_installed_ = true;
      }
      break;
    case kOnMouseMove:
      if (!on_mouse_move_installed_) {
        g_signal_connect(native, "motion-notify-event",
                         G_CALLBACK(OnMouseMove<T>), this);
        g_signal_connect(native, "enter-notify-event",
                         G_CALLBACK(OnMouseEvent<T>), this);
        g_signal_connect(native, "leave-notify-event",
                         G_CALLBACK(OnMouseEvent<T>), this);
        on_mouse_move_installed_ = true;
      }
      break;
    case kOnKey:
      if (!on_key_installed_) {
        g_signal_connect(native, "key-press-event",
                         G_CALLBACK(OnKeyDown<T>), this);
        g_signal_connect(native, "key-release-event",
                         G_CALLBACK(OnKeyUp<T>), this);
        on_key_installed_ = true;
      }
      break;
  }
}

// Explicit instantiation for Window and View.
template class Responder<Window>;
template class Responder<View>;

}  // namespace nu
