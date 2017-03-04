// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#include <gtk/gtk.h>

#include "nativeui/container.h"
#include "nativeui/gfx/geometry/rect_conversions.h"

namespace nu {

void View::PlatformDestroy() {
  if (view_) {
    gtk_widget_destroy(view_);
    g_object_unref(view_);
    view_ = nullptr;
  }
}

void View::TakeOverView(NativeView view) {
  view_ = view;
  if (view_) {
    g_object_ref_sink(view);
    gtk_widget_show(view);
  }
}

void View::SetBounds(const RectF& bounds) {
  return SetPixelBounds(ToEnclosingRect(bounds));
}

RectF View::GetBounds() const {
  return RectF(GetPixelBounds());
}

void View::SetPixelBounds(const Rect& bounds) {
  GdkRectangle rect = bounds.ToGdkRectangle();
  if (parent()) {
    // The size allocation is relative to the window instead of parent.
    GdkRectangle pb;
    gtk_widget_get_allocation(parent()->view(), &pb);
    rect.x += pb.x;
    rect.y += pb.y;
  }
  gtk_widget_size_allocate(view_, &rect);
}

Rect View::GetPixelBounds() const {
  GdkRectangle rect;
  gtk_widget_get_allocation(view_, &rect);
  if (parent()) {
    // The size allocation is relative to the window instead of parent.
    GdkRectangle pb;
    gtk_widget_get_allocation(parent()->view(), &pb);
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

void View::PlatformSetVisible(bool visible) {
  gtk_widget_set_visible(view_, visible);
}

bool View::IsVisible() const {
  return gtk_widget_get_visible(view_);
}

void View::PlatformSetBackgroundColor(Color color) {
  GdkRGBA rgba = color.ToGdkRGBA();
  gtk_widget_override_background_color(view_, GTK_STATE_FLAG_NORMAL, &rgba);
}

}  // namespace nu
