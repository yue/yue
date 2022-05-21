// Copyright 2017 Cheng Zhao. All rights reserved.
// Copyright 2013 GitHub, Inc.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/nu_window.h"

#include <objc/objc-runtime.h>

#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_responder.h"
#include "nativeui/window.h"

namespace nu {

namespace {

bool NUWindowInjected(NSWindow* self, SEL _cmd) {
  return true;
}

Responder* GetWindowShell(NSWindow* self, SEL _cmd) {
  return [self nuPrivate]->shell;
}

void EnableWindowTracking(NSWindow* self, SEL _cmd) {
  NUWindowPrivate* priv = [self nuPrivate];
  if (priv->tracking_area)
    return;
  NSTrackingAreaOptions trackingOptions = NSTrackingMouseEnteredAndExited |
                                          NSTrackingMouseMoved |
                                          NSTrackingActiveAlways |
                                          NSTrackingInVisibleRect;
  priv->tracking_area.reset([[NSTrackingArea alloc] initWithRect:NSZeroRect
                                                         options:trackingOptions
                                                           owner:self
                                                        userInfo:nil]);
  [[self contentView] addTrackingArea:priv->tracking_area.get()];
}

void DisableWindowTracking(NSWindow* self, SEL _cmd) {
  NUWindowPrivate* priv = [self nuPrivate];
  if (priv->tracking_area) {
    [[self contentView] removeTrackingArea:priv->tracking_area.get()];
    priv->tracking_area.reset();
  }
}

bool HasWindowTrackingArea(NSWindow* self, SEL _cmd) {
  return [self nuPrivate]->tracking_area;
}

void UpdateWindowTrackingAreas(NSWindow* self, SEL _cmd) {
  NUWindowPrivate* priv = [self nuPrivate];
  if (!priv->tracking_area)
    return;
  [self disableTracking];
  [self enableTracking];
}

void SetWindowStyle(NSWindow* self, SEL _cmd, NSUInteger style, bool yes) {
  bool maximizable = [[self standardWindowButton:NSWindowZoomButton] isEnabled];

  // Changing the styleMask of a frameless windows causes it to change size so
  // we explicitly disable resizing while setting it.
  NUWindowPrivate* priv = [self nuPrivate];
  priv->can_resize = false;

  if (yes)
    [self setStyleMask:[self styleMask] | style];
  else
    [self setStyleMask:[self styleMask] & ~style];

  priv->can_resize = true;

  // Change style mask will make the zoom button revert to default, probably
  // a bug of Cocoa.
  [[self standardWindowButton:NSWindowZoomButton] setEnabled:maximizable];
}

NSRect ConstrainFrameRect(NSWindow* self, SEL _cmd,
                          NSRect frameRect, NSScreen* screen) {
  if (![self nuPrivate]->can_resize)
    return [self frame];
  auto super_impl = reinterpret_cast<decltype(&ConstrainFrameRect)>(
      [[self superclass] instanceMethodForSelector:_cmd]);
  return super_impl(self, _cmd, frameRect, screen);
}

void SetFrameDisplay(NSWindow* self, SEL _cmd,
                     NSRect windowFrame, bool displayViews) {
  // constrainFrameRect is not called on hidden windows so disable adjusting
  // the frame directly when resize is disabled.
  if (![self nuPrivate]->can_resize)
    return;
  auto super_impl = reinterpret_cast<decltype(&SetFrameDisplay)>(
      [[self superclass] instanceMethodForSelector:_cmd]);
  super_impl(self, _cmd, windowFrame, displayViews);
}

}  // namespace

void InstallNUWindowMethods(Class cl) {
  if ([cl instancesRespondToSelector:@selector(nuInjected)])
    return;
  class_addMethod(cl, @selector(nuInjected), (IMP)NUWindowInjected, "B@:");

  class_addMethod(cl, @selector(shell), (IMP)GetWindowShell, "^v@:");
  class_addMethod(cl, @selector(enableTracking),
                  (IMP)EnableWindowTracking, "v@:");
  class_addMethod(cl, @selector(disableTracking),
                  (IMP)DisableWindowTracking, "v@:");
  class_addMethod(cl, @selector(hasTrackingArea),
                  (IMP)HasWindowTrackingArea, "B@:");
  class_addMethod(cl, @selector(updateTrackingAreas),
                  (IMP)UpdateWindowTrackingAreas, "v@:");
  class_addMethod(cl, @selector(setWindowStyle:on:),
                  (IMP)SetWindowStyle, "^v@:LB");

  class_addMethod(cl, @selector(constrainFrameRect:toScreen:),
                  (IMP)ConstrainFrameRect, "^{_NSRect=ffff}@:{_NSRect=ffff}@");
  class_addMethod(cl, @selector(setFrame:display:),
                  (IMP)SetFrameDisplay, "^v@:{_NSRect=ffff}B");
}

}  // namespace nu
