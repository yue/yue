// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

namespace nu {

Window::~Window() {
  gtk_widget_destroy(GTK_WIDGET(window_));
}

void Window::PlatformInit(const Options& options) {
  window_ = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gtk_window_set_default_size(window_,
                              options.content_bounds.width(),
                              options.content_bounds.height());
  gtk_window_move(window_, options.content_bounds.x(),
                           options.content_bounds.y());
}

void Window::PlatformSetContentView(Container* container) {
  GtkWidget* child = gtk_bin_get_child(GTK_BIN(window_));
  if (child)
    gtk_container_remove(GTK_CONTAINER(window_), child);
  gtk_container_add(GTK_CONTAINER(window_), container->view());
}

gfx::Rect Window::ContentBoundsToWindowBounds(const gfx::Rect& bounds) const {
  return bounds;
}

gfx::Rect Window::WindowBoundsToContentBounds(const gfx::Rect& bounds) const {
  return bounds;
}

void Window::SetBounds(const gfx::Rect& bounds) {
  gtk_window_move(window_, bounds.x(), bounds.y());
  gtk_window_resize(window_, bounds.width(), bounds.height());

  // GTK+ does not allocate size when window is hidden.
  if (!IsVisible())
    GetContentView()->SetBounds(gfx::Rect(gfx::Point(), bounds.size()));
}

gfx::Rect Window::GetBounds() const {
  int x, y, width, height;
  gtk_window_get_position(window_, &x, &y);
  gtk_window_get_size(window_, &width, &height);
  return gfx::Rect(x, y, width, height);
}

void Window::SetVisible(bool visible) {
  if (visible) {
    gtk_window_set_focus_on_map(window_, false);
    gtk_widget_set_visible(GTK_WIDGET(window_), visible);
    gtk_window_set_focus_on_map(window_, true);
  } else {
    gtk_widget_set_visible(GTK_WIDGET(window_), visible);
  }
}

bool Window::IsVisible() const {
  return gtk_widget_get_visible(GTK_WIDGET(window_));
}

}  // namespace nu
