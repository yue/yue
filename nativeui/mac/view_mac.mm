// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/view_mac.h"

#include "nativeui/container.h"
#include "nativeui/gfx/geometry/point_conversions.h"
#include "nativeui/gfx/geometry/rect_conversions.h"
#include "nativeui/gfx/mac/painter_mac.h"
#include "nativeui/mac/events_handler.h"
#include "nativeui/mac/mouse_capture.h"
#include "nativeui/mac/nu_private.h"

namespace nu {

namespace {

// There is no way to know when another application has installed an event
// monitor, we have to assume only current app can capture view.
View* g_captured_view = nullptr;

}  // namespace

void View::PlatformDestroy() {
  [view_ release];
}

void View::TakeOverView(NativeView view) {
  view_ = view;

  if (!IsNUView(view))
    return;

  // Install events handle for the view's class.
  Class cl = [view class];
  if (!EventHandlerInstalled(cl)) {
    AddMouseEventHandlerToClass(cl);
    AddKeyEventHandlerToClass(cl);
    AddViewMethodsToClass(cl);
  }

  // Initialize private bits of the view.
  NUPrivate* priv = [view nuPrivate];
  priv->shell = this;

  // Set the focusable property to the parent class's default one.
  SEL cmd = @selector(acceptsFirstResponder);
  auto super_impl = reinterpret_cast<BOOL (*)(NSView*, SEL)>(
      [[view superclass] instanceMethodForSelector:cmd]);
  priv->focusable = super_impl(view, cmd);

  // Install event tracking area.
  [view enableTracking];
}

void View::SetBounds(const RectF& bounds) {
  NSRect frame = bounds.ToCGRect();
  [view_ setFrame:frame];
  // Calling setFrame manually does not trigger adjustSubviews.
  [view_ resizeSubviewsWithOldSize:frame.size];
}

Vector2dF View::OffsetFromView(const View* from) const {
  NSPoint point = [view_ convertPoint:NSZeroPoint toView:from->view_];
  return Vector2dF(point.x, point.y);
}

Vector2dF View::OffsetFromWindow() const {
  NSPoint point = [view_ convertPoint:NSZeroPoint toView:nil];
  return Vector2dF(point.x, point.y);
}

RectF View::GetBounds() const {
  return RectF([view_ frame]);
}

void View::SetPixelBounds(const Rect& bounds) {
  SetBounds(RectF(bounds));
}

Rect View::GetPixelBounds() const {
  return ToNearestRect(GetBounds());
}

void View::SchedulePaint() {
  [view_ setNeedsDisplay:YES];
}

void View::PlatformSetVisible(bool visible) {
  [view_ setHidden:!visible];
}

bool View::IsVisible() const {
  return ![view_ isHidden];
}

void View::Focus() {
  if (view_.window && IsFocusable())
    [view_.window makeFirstResponder:view_];
}

bool View::HasFocus() const {
  if (view_.window)
    return view_.window.firstResponder == view_;
  else
    return false;
}

void View::SetFocusable(bool focusable) {
  NUPrivate* priv = [view_ nuPrivate];
  priv->focusable = focusable;
}

bool View::IsFocusable() const {
  return [view_ acceptsFirstResponder];
}

void View::SetCapture() {
  if (g_captured_view)
    g_captured_view->ReleaseCapture();

  NUPrivate* priv = [view_ nuPrivate];
  priv->mouse_capture.reset(new MouseCapture(this));
  g_captured_view = this;
}

void View::ReleaseCapture() {
  if (g_captured_view != this)
    return;

  NUPrivate* priv = [view_ nuPrivate];
  priv->mouse_capture.reset();
  g_captured_view = nullptr;
  on_capture_lost.Emit(this);
}

bool View::HasCapture() const {
  return g_captured_view == this;
}

void View::PlatformSetBackgroundColor(Color color) {
  if (IsNUView(view_))
    [view_ setNUBackgroundColor:color];
}

}  // namespace nu
