// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#include "nativeui/container.h"

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

void View::SetBounds(const Rect& bounds) {
  GdkRectangle rect = bounds.ToGdkRectangle();
  if (parent()) {
    // The size allocation is relative to the window instead of parent.
    Point pb = parent()->GetWindowOrigin();
    rect.x += pb.x();
    rect.y += pb.y();
  }
  gtk_widget_size_allocate(view_, &rect);
}

Rect View::GetBounds() const {
  GdkRectangle rect;
  gtk_widget_get_allocation(view_, &rect);
  if (parent()) {
    // The size allocation is relative to the window instead of parent.
    Point pb = parent()->GetWindowOrigin();
    rect.x -= pb.x();
    rect.y -= pb.y();
  }
  Rect bounds = Rect(rect);
  // GTK uses (-1, -1, 1, 1) and (0, 0, 1, 1) as empty bounds, we should match
  // the behavior of other platforms by returning an empty rect.
  if (bounds == Rect(-1, -1, 1, 1) || bounds == Rect(0, 0, 1, 1))
    return Rect();
  return bounds;
}

void View::SetPixelBounds(const Rect& bounds) {
  SetBounds(bounds);
}

Rect View::GetPixelBounds() const {
  return GetBounds();
}

Point View::GetWindowOrigin() const {
  GdkRectangle rect;
  gtk_widget_get_allocation(view_, &rect);
  return Point(rect.x, rect.y);
}

Point View::GetWindowPixelOrigin() const {
  return GetWindowOrigin();
}

bool View::SetDefaultStyle(const Size& size) {
  return DoSetDefaultStyle(size);
}

bool View::SetPixelPreferredSize(const Size& size) {
  return DoSetDefaultStyle(size);
}

int View::DIPToPixel(int length) const {
  return length;
}

void View::PlatformSetVisible(bool visible) {
  gtk_widget_set_visible(view_, visible);
}

bool View::IsVisible() const {
  return gtk_widget_get_visible(view_);
}

}  // namespace nu
