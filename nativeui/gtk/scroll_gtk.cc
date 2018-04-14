// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/scroll.h"

#include <gtk/gtk.h>

#include "nativeui/gtk/widget_util.h"

namespace nu {

namespace {

GtkPolicyType PolicyToGTK(Scroll::Policy policy) {
  if (policy == Scroll::Policy::Always)
    return GTK_POLICY_ALWAYS;
  else if (policy == Scroll::Policy::Never)
    return GTK_POLICY_NEVER;
  else
    return GTK_POLICY_AUTOMATIC;
}

Scroll::Policy PolicyFromGTK(GtkPolicyType policy) {
  if (policy == GTK_POLICY_ALWAYS)
    return Scroll::Policy::Always;
  else if (policy == GTK_POLICY_NEVER)
    return Scroll::Policy::Never;
  else
    return Scroll::Policy::Automatic;
}

}  // namespace

void Scroll::PlatformInit() {
  TakeOverView(gtk_scrolled_window_new(nullptr, nullptr));
  GtkWidget* viewport = gtk_viewport_new(
      gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(GetNative())),
      gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(GetNative())));
  gtk_widget_show(viewport);
  gtk_container_add(GTK_CONTAINER(GetNative()), viewport);
}

void Scroll::PlatformSetContentView(View* view) {
  // Receive the content size from current content view.
  Size csize = view->GetPixelBounds().size();
  if (content_view_) {
    int w, h;
    gtk_widget_get_size_request(content_view_->GetNative(), &w, &h);
    csize = Size(w, h);
  }

  GtkWidget* viewport = gtk_bin_get_child(GTK_BIN(GetNative()));
  GtkWidget* child = gtk_bin_get_child(GTK_BIN(viewport));
  if (child) {
    gtk_container_remove(GTK_CONTAINER(viewport), child);
    gtk_widget_set_size_request(child, -1, -1);
  }

  gtk_container_add(GTK_CONTAINER(viewport), view->GetNative());
  gtk_widget_set_size_request(view->GetNative(), csize.width(), csize.height());
}

void Scroll::SetContentSize(const SizeF& size) {
  GetContentView()->SetBounds(RectF(size));
  // Viewport calculates the content view according to child's size request.
  gtk_widget_set_size_request(GetContentView()->GetNative(),
                              size.width(), size.height());
  GetContentView()->Layout();
  // Scroll to top-left after setting content size.
  auto* h_adjust = gtk_scrolled_window_get_hadjustment(
      GTK_SCROLLED_WINDOW(GetNative()));
  gtk_adjustment_set_value(h_adjust, gtk_adjustment_get_lower(h_adjust));
  auto* v_adjust = gtk_scrolled_window_get_vadjustment(
      GTK_SCROLLED_WINDOW(GetNative()));
  gtk_adjustment_set_value(v_adjust, gtk_adjustment_get_lower(v_adjust));
}

void Scroll::SetOverlayScrollbar(bool overlay) {
  if (GtkVersionCheck(3, 16))
    gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(GetNative()),
                                              overlay);
}

bool Scroll::IsOverlayScrollbar() const {
  if (GtkVersionCheck(3, 16)) {
    return gtk_scrolled_window_get_overlay_scrolling(
        GTK_SCROLLED_WINDOW(GetNative()));
  } else {
    return false;
  }
}

void Scroll::SetScrollbarPolicy(Policy h_policy, Policy v_policy) {
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(GetNative()),
                                 PolicyToGTK(h_policy), PolicyToGTK(v_policy));
}

std::tuple<Scroll::Policy, Scroll::Policy> Scroll::GetScrollbarPolicy() const {
  GtkPolicyType hp, vp;
  gtk_scrolled_window_get_policy(GTK_SCROLLED_WINDOW(GetNative()), &hp, &vp);
  return std::make_tuple(PolicyFromGTK(hp), PolicyFromGTK(vp));
}

}  // namespace nu
