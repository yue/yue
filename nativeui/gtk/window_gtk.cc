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

}  // namespace

void Window::PlatformInit(const Options& options) {
  window_ = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
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
}

void Window::PlatformSetMenuBar(MenuBar* menu_bar) {
  // TODO(zcbenz): Content bound should take account of menubar.
  GtkContainer* vbox = GTK_CONTAINER(gtk_bin_get_child(GTK_BIN(window_)));
  if (menu_bar_)
    gtk_container_remove(vbox, GTK_WIDGET(menu_bar_->menu()));
  GtkWidget* menu = GTK_WIDGET(menu_bar->menu());
  gtk_container_add(vbox, menu);
  gtk_box_set_child_packing(GTK_BOX(vbox), menu, FALSE, FALSE, 0,
                            GTK_PACK_START);
}

void Window::SetContentBounds(const RectF& bounds) {
  SetBounds(bounds);
}

RectF Window::GetContentBounds() const {
  return GetBounds();
}

void Window::SetBounds(const RectF& bounds) {
  gtk_window_move(window_, bounds.x(), bounds.y());
  gtk_window_resize(window_, bounds.width(), bounds.height());
}

RectF Window::GetBounds() const {
  int x, y, width, height;
  gtk_window_get_position(window_, &x, &y);
  gtk_window_get_size(window_, &width, &height);
  return RectF(x, y, width, height);
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
