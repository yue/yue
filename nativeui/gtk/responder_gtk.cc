// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/responder.h"

#include <gtk/gtk.h>

#include "base/notreached.h"
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

// Store pending tasks of sending mouse-leave events.
struct MouseLeaveTask {
  MessageLoop::TimerId timer = 0;
  Responder* responder = nullptr;
  GtkWidget* widget = nullptr;
};
std::vector<MouseLeaveTask> g_mouse_leave_tasks;

// Used to retrigger a pending mouse-leave event that was cancelled by child.
void RetriggerPendingMouseLeave(GtkWidget* widget, const MouseEvent& event) {
  void* data = g_object_get_data(G_OBJECT(widget), "pending-mouse-leave");
  if (!data)
    return;
  auto* responder = static_cast<Responder*>(data);
  responder->on_mouse_leave.Emit(responder, event);
  // Might be nested views.
  RetriggerPendingMouseLeave(responder->GetNative(), event);
  // Clear.
  g_object_set_data(G_OBJECT(widget), "pending-mouse-leave", nullptr);
}

// Used to delay the emit of mouse-leave event.
void DelayedOnMouseLeave(scoped_refptr<Responder> responder,
                         const MouseEvent& event) {
  responder->on_mouse_leave.Emit(responder.get(), event);
  RetriggerPendingMouseLeave(responder->GetNative(), event);
  // Remove the task.
  for (auto iter = g_mouse_leave_tasks.begin();
       iter != g_mouse_leave_tasks.end(); ++iter) {
    if (iter->responder == responder.get()) {
      g_mouse_leave_tasks.erase(iter);
      break;
    }
  }
}

// Utility to call Release of a RefCounted.
template<typename T>
void ReleaseRefCounted(void* ptr) {
  static_cast<T*>(ptr)->Release();
}

gboolean OnMouseEvent(GtkWidget* widget, GdkEvent* event,
                      Responder* responder) {
  switch (event->any.type) {
    case GDK_BUTTON_PRESS: {
      return responder->on_mouse_down.Emit(responder,
                                           MouseEvent(event, widget));
    }
    case GDK_BUTTON_RELEASE: {
      return responder->on_mouse_up.Emit(responder, MouseEvent(event, widget));
    }
    case GDK_ENTER_NOTIFY: {
      for (auto iter = g_mouse_leave_tasks.begin();
           iter != g_mouse_leave_tasks.end(); ++iter) {
        if (responder != iter->responder &&
            !gtk_widget_is_ancestor(widget, iter->widget))
          continue;
        // If there is a pending mosue leave event in this view or in its
        // parent, clear it.
        MessageLoop::ClearTimeout(iter->timer);
        g_mouse_leave_tasks.erase(iter);
        // If the pending mouse-leave event comes from this view, ignore this
        // mouse-enter event since the view is still considered as hovered.
        if (responder == iter->responder)
          return false;
        // When a child cancels its parent's mouse-leave event, reschedule
        // a new one when the child's mouse-leave event emits.
        iter->responder->AddRef();
        g_object_set_data_full(G_OBJECT(widget),
                               "pending-mouse-leave",
                               iter->responder,
                               ReleaseRefCounted<Responder>);
        break;
      }
      responder->on_mouse_enter.Emit(responder, MouseEvent(event, widget));
      return false;
    }
    case GDK_LEAVE_NOTIFY: {
      // In GTK when mouse enters the child window it will be considered
      // as leaving the parent window and will emit mouse-leave event in
      // parent. This is different from other platforms and causes major
      // bugs in apps.
      // We work around it by delaying the mouse leave to next tick, and
      // if there is a mouse enter event happening in child view, the event
      // will be rescheduled until the mouse leaves the child view.
      if (responder->GetType() == Responder::Type::View &&
          static_cast<View*>(responder)->IsContainer() &&
          static_cast<Container*>(responder)->ChildCount() > 0) {
        // Do not push duplicate mouse-leave events.
        for (const auto& task : g_mouse_leave_tasks) {
          if (task.responder == responder) {
            NOTREACHED();
            return false;
          }
        }
        // Push a delayed task.
        g_mouse_leave_tasks.push_back({
            MessageLoop::SetTimeout(
              0, std::bind(DelayedOnMouseLeave,
                           responder, MouseEvent(event, widget))),
            responder,
            widget,
        });
        return false;
      }
      MouseEvent mouse_event(event, widget);
      responder->on_mouse_leave.Emit(responder, mouse_event);
      RetriggerPendingMouseLeave(widget, mouse_event);
      return false;
    }
    default: {
      return false;
    }
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
