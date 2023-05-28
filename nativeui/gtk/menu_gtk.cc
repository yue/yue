// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu.h"

#include <gtk/gtk.h>

#include "base/notreached.h"

namespace nu {

namespace {

void OnMenuHidden(GtkWidget* widget, void*) {
  gtk_main_quit();
}

void BlockUntilMenuIsHidden(GtkMenuShell* menu) {
  gint id = g_signal_connect(menu, "hide", G_CALLBACK(OnMenuHidden), nullptr);
  gtk_main();
  g_signal_handler_disconnect(menu, id);
}

}  // namespace

Menu::Menu() : MenuBase(GTK_MENU_SHELL(gtk_menu_new())) {}

void Menu::Popup() {
  gtk_menu_popup(GTK_MENU(GetNative()), nullptr, nullptr, nullptr, nullptr,
                 0, gtk_get_current_event_time());
  BlockUntilMenuIsHidden(GetNative());
}

void Menu::PopupAt(const PointF& point) {
  GdkEvent* current = gtk_get_current_event();
  if (!current) {
    NOTREACHED() << "Can not find current event";
    return;
  }
  // Use the current directly if we are current processing mouse event.
  bool managed = true;
  if (current->type == GDK_BUTTON_PRESS ||
      current->type == GDK_BUTTON_RELEASE) {
    managed = false;
  }
  GdkEvent* event = managed ? gdk_event_new(GDK_BUTTON_PRESS) : current;
  // Copy necessary properties.
  if (managed) {
    event->button.send_event = false;
    event->button.window = gdk_event_get_window(current);
    event->button.time = gdk_event_get_time(current);
  }
  // Set mouse position.
  gint x = static_cast<gint>(point.x());
  gint y = static_cast<gint>(point.y());
  event->button.x = x;
  event->button.y = y;
  gint origin_x, origin_y;
  gdk_window_get_origin(event->button.window, &origin_x, &origin_y);
  GdkRectangle rect = { x - origin_x, y- origin_y, 1, 1 };
  // Popup and block.
  GdkWindow* window = gdk_event_get_window(current);
  gtk_menu_popup_at_rect(GTK_MENU(GetNative()), window, &rect,
                         GDK_GRAVITY_SOUTH_EAST, GDK_GRAVITY_NORTH_WEST, event);
  BlockUntilMenuIsHidden(GetNative());
  if (managed)
    gdk_event_free(event);
  gdk_event_free(current);
}

}  // namespace nu
