// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#include "nativeui/menu_bar.h"

namespace nu {

namespace {

// User clicks the close button.
gboolean OnClose(GtkWidget *widget, GdkEvent *event, Window* window) {
  if (window->should_close.is_null() || window->should_close.Run())
    window->Close();

  // We are destroying the window ourselves, so prevent the default behavior.
  return TRUE;
}

// Force window to allocate size for content view.
void ForceSizeAllocation(GtkWindow* window, GtkWidget* view) {
  GdkRectangle rect = { 0, 0 };
  gtk_window_get_size(window, &rect.width, &rect.height);
  gtk_widget_size_allocate(view, &rect);
}

}  // namespace

void Window::PlatformInit(const Options& options) {
  window_ = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gtk_window_add_accel_group(window_, accel_manager_.accel_group());
  gtk_window_set_focus_on_map(window_, false);

  // Must use a vbox to pack menubar.
  GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_container_add(GTK_CONTAINER(window_), vbox);

  if (!options.bounds.IsEmpty()) {
    gtk_window_set_default_size(window_, options.bounds.width(),
                                         options.bounds.height());
    gtk_window_move(window_, options.bounds.x(), options.bounds.y());
  }

  g_signal_connect(window_, "delete-event", G_CALLBACK(OnClose), this);
}

void Window::PlatformDestroy() {
  if (window_)
    gtk_widget_destroy(GTK_WIDGET(window_));
}

void Window::Close() {
  if (!should_close.is_null() && !should_close.Run())
    return;

  on_close.Emit();
  gtk_widget_destroy(GTK_WIDGET(window_));

  window_ = nullptr;
}

void Window::PlatformSetContentView(Container* container) {
  GtkContainer* vbox = GTK_CONTAINER(gtk_bin_get_child(GTK_BIN(window_)));
  if (content_view_)
    gtk_container_remove(vbox, content_view_->view());
  gtk_container_add(vbox, container->view());
  gtk_box_set_child_packing(GTK_BOX(vbox), container->view(), TRUE, TRUE,
                            0, GTK_PACK_END);

  ForceSizeAllocation(window_, container->view());
}

void Window::PlatformSetMenuBar(MenuBar* menu_bar) {
  GtkContainer* vbox = GTK_CONTAINER(gtk_bin_get_child(GTK_BIN(window_)));
  if (menu_bar_)
    gtk_container_remove(vbox, GTK_WIDGET(menu_bar_->menu()));
  GtkWidget* menu = GTK_WIDGET(menu_bar->menu());
  gtk_container_add(vbox, menu);
  gtk_box_set_child_packing(GTK_BOX(vbox), menu, FALSE, FALSE, 0,
                            GTK_PACK_START);

  ForceSizeAllocation(window_, content_view_->view());
}

void Window::SetContentBounds(const RectF& bounds) {
  RectF window_bounds(bounds);
  GdkWindow* gdkwindow = gtk_widget_get_window(GTK_WIDGET(window_));
  if (gdkwindow) {
    // Get frame size.
    GdkRectangle rect;
    gdk_window_get_frame_extents(gdkwindow, &rect);
    // The position of window without frame, which includes menubar.
    int x, y;
    gdk_window_get_position(gdkwindow, &x, &y);
    window_bounds.Offset(rect.x - x, rect.y - y);
  }
  // The position of menubar.
  GdkRectangle alloc;
  gtk_widget_get_allocation(content_view_->view(), &alloc);
  window_bounds.Inset(-alloc.x, -alloc.y, 0, 0);

  gtk_window_resize(window_, window_bounds.width(), window_bounds.height());
  gtk_window_move(window_, window_bounds.x(), window_bounds.y());
  ForceSizeAllocation(window_, content_view_->view());
}

RectF Window::GetContentBounds() const {
  GdkWindow* gdkwindow = gtk_widget_get_window(GTK_WIDGET(window_));
  // The position of window without frame, which includes menubar.
  int x, y;
  if (gdkwindow)
    gdk_window_get_position(gdkwindow, &x, &y);
  else
    gtk_window_get_position(window_, &x, &y);
  // The relative position of content view.
  GdkRectangle alloc;
  gtk_widget_get_allocation(content_view_->view(), &alloc);
  return RectF(x + alloc.x, y + alloc.y, alloc.width, alloc.height);
}

void Window::SetBounds(const RectF& bounds) {
  SizeF window_size(bounds.size());
  GdkWindow* gdkwindow = gtk_widget_get_window(GTK_WIDGET(window_));
  if (gdkwindow) {
    // Get frame size.
    GdkRectangle rect;
    gdk_window_get_frame_extents(gdkwindow, &rect);
    // Subtract gdk window size to get frame insets.
    window_size.Enlarge(-(rect.width - gdk_window_get_width(gdkwindow)),
                        -(rect.height - gdk_window_get_height(gdkwindow)));
  }

  gtk_window_resize(window_, window_size.width(), window_size.height());
  gtk_window_move(window_, bounds.x(), bounds.y());
  ForceSizeAllocation(window_, content_view_->view());
}

RectF Window::GetBounds() const {
  GdkWindow* gdkwindow = gtk_widget_get_window(GTK_WIDGET(window_));
  if (gdkwindow) {
    GdkRectangle rect;
    gdk_window_get_frame_extents(gdkwindow, &rect);
    return RectF(Rect(rect));
  } else {
    int x, y, width, height;
    gtk_window_get_position(window_, &x, &y);
    gtk_window_get_size(window_, &width, &height);
    return RectF(x, y, width, height);
  }
}

void Window::SetVisible(bool visible) {
  gtk_widget_set_visible(GTK_WIDGET(window_), visible);
}

bool Window::IsVisible() const {
  return gtk_widget_get_visible(GTK_WIDGET(window_));
}

void Window::SetBackgroundColor(Color color) {
  GdkRGBA gcolor = color.ToGdkRGBA();
  gtk_widget_override_background_color(GTK_WIDGET(window_),
                                       GTK_STATE_FLAG_NORMAL, &gcolor);
}

}  // namespace nu
