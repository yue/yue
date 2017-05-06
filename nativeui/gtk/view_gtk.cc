// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#include <gtk/gtk.h>

#include "nativeui/container.h"
#include "nativeui/events/event.h"
#include "nativeui/gfx/geometry/point_f.h"
#include "nativeui/gfx/geometry/rect_conversions.h"
#include "nativeui/gfx/geometry/rect_f.h"
#include "nativeui/gtk/nu_container.h"

namespace nu {

namespace {

// The view that has the capture.
View* g_grabbed_view = nullptr;

void OnSizeAllocate(GtkWidget* widget, GdkRectangle* allocation, View* view) {
  // Ignore empty sizes on initialization.
  if (allocation->x == -1 && allocation->y == -1 &&
      allocation->width == 1 && allocation->height == 1)
    return;

  view->OnSizeChanged();
}

gboolean OnMouseMove(GtkWidget* widget, GdkEvent* event, View* view) {
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

  // Otherwise dispatch the event.
  if (!view->on_mouse_move.IsEmpty()) {
    view->on_mouse_move.Emit(view, MouseEvent(event, widget));
    return false;
  }

  return false;
}

gboolean OnMouseEvent(GtkWidget* widget, GdkEvent* event, View* view) {
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

gboolean OnKeyDown(GtkWidget* widget, GdkEvent* event, View* view) {
  return view->on_key_down.Emit(view, KeyEvent(event, widget));
}

gboolean OnKeyUp(GtkWidget* widget, GdkEvent* event, View* view) {
  return view->on_key_up.Emit(view, KeyEvent(event, widget));
}

}  // namespace

void View::PlatformDestroy() {
  if (view_) {
    gtk_widget_destroy(view_);
    g_object_unref(view_);
    // The PlatformDestroy might be called for multiple times, see
    // Container::PlatformDestroy for more about this.
    view_ = nullptr;
  }
}

void View::TakeOverView(NativeView view) {
  view_ = view;
  g_object_ref_sink(view);
  gtk_widget_show(view);  // visible by default

  // Make the view accepts events.
  gtk_widget_add_events(view, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                              GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

  // Install event hooks.
  g_signal_connect(view, "size-allocate", G_CALLBACK(OnSizeAllocate), this);
  g_signal_connect(view, "motion-notify-event", G_CALLBACK(OnMouseMove), this);
  // TODO(zcbenz): Lazily install the event hooks.
  g_signal_connect(view, "button-press-event", G_CALLBACK(OnMouseEvent), this);
  g_signal_connect(view, "button-release-event", G_CALLBACK(OnMouseEvent), this);  // NOLINT
  g_signal_connect(view, "enter-notify-event", G_CALLBACK(OnMouseEvent), this);
  g_signal_connect(view, "leave-notify-event", G_CALLBACK(OnMouseEvent), this);
  g_signal_connect(view, "key-press-event", G_CALLBACK(OnKeyDown), this);
  g_signal_connect(view, "key-release-event", G_CALLBACK(OnKeyUp), this);
}

Vector2dF View::OffsetFromView(const View* from) const {
  GdkRectangle rect_f;
  gtk_widget_get_allocation(from->GetNative(), &rect_f);
  GdkRectangle rect_d;
  gtk_widget_get_allocation(GetNative(), &rect_d);
  return Vector2dF(rect_d.x - rect_f.x, rect_d.y - rect_f.y);
}

Vector2dF View::OffsetFromWindow() const {
  GdkRectangle rect;
  gtk_widget_get_allocation(GetNative(), &rect);
  return Vector2dF(rect.x, rect.y);
}

void View::SetBounds(const RectF& bounds) {
  return SetPixelBounds(ToNearestRect(bounds));
}

RectF View::GetBounds() const {
  return RectF(GetPixelBounds());
}

void View::SetPixelBounds(const Rect& bounds) {
  // The size allocation is relative to the window instead of parent.
  GdkRectangle rect = bounds.ToGdkRectangle();
  if (GetParent()) {
    GdkRectangle pb;
    gtk_widget_get_allocation(GetParent()->GetNative(), &pb);
    rect.x += pb.x;
    rect.y += pb.y;
  }

  // Call get_preferred_width before size allocation, otherwise GTK would print
  // warnings like "How does the code know the size to allocate?".
  gint width;
  gtk_widget_get_preferred_width(view_, &width, nullptr);

  gtk_widget_size_allocate(view_, &rect);
}

Rect View::GetPixelBounds() const {
  GdkRectangle rect;
  gtk_widget_get_allocation(view_, &rect);
  if (GetParent()) {
    // The size allocation is relative to the window instead of parent.
    GdkRectangle pb;
    gtk_widget_get_allocation(GetParent()->GetNative(), &pb);
    rect.x -= pb.x;
    rect.y -= pb.y;
  }
  Rect bounds = Rect(rect);
  // GTK uses (-1, -1, 1, 1) and (0, 0, 1, 1) as empty bounds, we should match
  // the behavior of other platforms by returning an empty rect.
  if (bounds == Rect(-1, -1, 1, 1) || bounds == Rect(0, 0, 1, 1))
    return Rect();
  return bounds;
}

void View::SchedulePaint() {
  gtk_widget_queue_draw(view_);
}

void View::PlatformSetVisible(bool visible) {
  gtk_widget_set_visible(view_, visible);
}

bool View::IsVisible() const {
  return gtk_widget_get_visible(view_);
}

void View::Focus() {
  gtk_widget_grab_focus(view_);
}

bool View::HasFocus() const {
  return gtk_widget_is_focus(view_);
}

void View::SetFocusable(bool focusable) {
  gtk_widget_set_can_focus(view_, focusable);
}

bool View::IsFocusable() const {
  return gtk_widget_get_can_focus(view_);
}

void View::SetCapture() {
  // Get the GDK window.
  GdkWindow* window;
  if (GetClassName() == Container::kClassName)
    window = nu_container_get_window(NU_CONTAINER(view_));
  else
    window = gtk_widget_get_window(view_);
  if (!window)
    return;

  const GdkEventMask mask = GdkEventMask(GDK_BUTTON_PRESS_MASK |
                                         GDK_BUTTON_RELEASE_MASK |
                                         GDK_POINTER_MOTION_HINT_MASK |
                                         GDK_POINTER_MOTION_MASK);
  if (gdk_pointer_grab(window, FALSE, mask, NULL, NULL,
                       GDK_CURRENT_TIME) == GDK_GRAB_SUCCESS)
    g_grabbed_view = this;
}

void View::ReleaseCapture() {
  gdk_pointer_ungrab(GDK_CURRENT_TIME);

  // In X11 the grab can not be hijacked by other applications, so the only
  // possible case for losing capture is to call this function.
  if (g_grabbed_view) {
    g_grabbed_view->on_capture_lost.Emit(g_grabbed_view);
    g_grabbed_view = nullptr;
  }
}

bool View::HasCapture() const {
  return gdk_pointer_is_grabbed() && g_grabbed_view == this;
}

void View::SetMouseDownCanMoveWindow(bool yes) {
  g_object_set_data(G_OBJECT(view_), "draggable", yes ? this : nullptr);
}

bool View::IsMouseDownCanMoveWindow() const {
  return g_object_get_data(G_OBJECT(view_), "draggable");
}

void View::SetBackgroundColor(Color color) {
  GdkRGBA rgba = color.ToGdkRGBA();
  gtk_widget_override_background_color(view_, GTK_STATE_FLAG_NORMAL, &rgba);
}

}  // namespace nu
