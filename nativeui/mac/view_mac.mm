// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/nu_view.h"

#include "nativeui/browser.h"
#include "nativeui/container.h"
#include "nativeui/gfx/font.h"
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
  if (!NUViewMethodsInstalled(cl)) {
    InstallNUViewMethods(cl);
    // TODO(zcbenz): Lazily install the event hooks.
    if (GetClassName() != Browser::kClassName) {
      // WKWebView does not like having mouse event handlers installed.
      AddMouseEventHandlerToClass(cl);
    }
    AddKeyEventHandlerToClass(cl);
  }

  // Initialize private bits of the view.
  NUPrivate* priv = [view nuPrivate];
  priv->shell = this;

  // Set the |focusable| property to the parent class's default one.
  SEL cmd = @selector(acceptsFirstResponder);
  auto super_impl = reinterpret_cast<BOOL (*)(NSView*, SEL)>(
      [[view superclass] instanceMethodForSelector:cmd]);
  priv->focusable = super_impl(view, cmd);

  // Set the |draggable| property to the parent class's default one.
  cmd = @selector(mouseDownCanMoveWindow);
  super_impl = reinterpret_cast<BOOL (*)(NSView*, SEL)>(
      [[view superclass] instanceMethodForSelector:cmd]);
  priv->draggable = super_impl(view, cmd);

  // Install event tracking area.
  // TODO(zcbenz): Lazily install the event hooks.
  [view enableTracking];
}

void View::SetBounds(const RectF& bounds) {
  NSRect frame = bounds.ToCGRect();
  [view_ setFrame:frame];
  // Calling setFrame manually does not trigger resizeSubviewsWithOldSize.
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

void View::SchedulePaintRect(const RectF& rect) {
  [view_ setNeedsDisplayInRect:rect.ToCGRect()];
}

void View::PlatformSetVisible(bool visible) {
  [view_ setHidden:!visible];
}

bool View::IsVisible() const {
  return ![view_ isHidden];
}

void View::SetEnabled(bool enable) {
  [view_ setNUEnabled:enable];
}

bool View::IsEnabled() const {
  return [view_ isNUEnabled];
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

void View::SetMouseDownCanMoveWindow(bool yes) {
  NUPrivate* priv = [view_ nuPrivate];
  priv->draggable = yes;

  // AppKit will not update its cache of mouseDownCanMoveWindow unless something
  // changes.
  [[view_ window] setMovableByWindowBackground:NO];
  [[view_ window] setMovableByWindowBackground:YES];
}

bool View::IsMouseDownCanMoveWindow() const {
  return [view_ mouseDownCanMoveWindow];
}

void View::PlatformSetFont(Font* font) {
  if (IsNUView(view_))
    [view_ setNUFont:font];
}

void View::SetColor(Color color) {
  if (IsNUView(view_))
    [view_ setNUColor:color];
}

void View::SetBackgroundColor(Color color) {
  if (IsNUView(view_))
    [view_ setNUBackgroundColor:color];
}

void View::SetWantsLayer(bool wants) {
  [view_ nuPrivate]->wants_layer = wants;
  [view_ setWantsLayer:wants];
}

bool View::WantsLayer() const {
  return [view_ wantsLayer];
}

}  // namespace nu
