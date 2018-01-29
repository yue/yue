// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/nu_view.h"

#include <objc/objc-runtime.h>

#include "base/mac/mac_util.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_window.h"
#include "nativeui/window.h"

namespace nu {

namespace {

// Returns whether the view belongs to a frameless window.
bool IsFramelessWindow(NSView* view) {
  if (![view window])
    return false;
  if (![[view window] respondsToSelector:@selector(shell)])
    return false;
  return ![static_cast<NUWindow*>([view window]) shell]->HasFrame();
}

// Following methods are overrided in NUView.

bool NUInjected(NSView* self, SEL _cmd) {
  return true;
}

View* GetShell(NSView* self, SEL _cmd) {
  return [self nuPrivate]->shell;
}

BOOL AcceptsFirstResponder(NSView* self, SEL _cmd) {
  return [self nuPrivate]->focusable;
}

BOOL MouseDownCanMoveWindow(NSView* self, SEL _cmd) {
  return [self nuPrivate]->draggable;
}

void EnableTracking(NSView* self, SEL _cmd) {
  NUPrivate* priv = [self nuPrivate];
  NSTrackingAreaOptions trackingOptions = NSTrackingMouseEnteredAndExited |
                                          NSTrackingMouseMoved |
                                          NSTrackingActiveAlways |
                                          NSTrackingInVisibleRect;
  priv->tracking_area.reset([[NSTrackingArea alloc] initWithRect:NSZeroRect
                                                         options:trackingOptions
                                                           owner:self
                                                        userInfo:nil]);
  [self addTrackingArea:priv->tracking_area.get()];
}

void DisableTracking(NSView* self, SEL _cmd) {
  NUPrivate* priv = [self nuPrivate];
  if (priv->tracking_area) {
    [self removeTrackingArea:priv->tracking_area.get()];
    priv->tracking_area.reset();
  }
}

// Following methods are overrided in NUView to make sure that content view
// of frameless always takes the size of its parent view.

// This method is directly called by NSWindow during a window resize on OSX
// 10.10.0, beta 2. We must override it to prevent the content view from
// shrinking.
void SetFrameSize(NSView* self, SEL _cmd, NSSize size) {
  if (IsFramelessWindow(self) &&
      [self nuPrivate]->is_content_view && [self superview])
    size = [[self superview] bounds].size;

  NSSize old_size = [self bounds].size;

  auto super_impl = reinterpret_cast<decltype(&SetFrameSize)>(
      [[self superclass] instanceMethodForSelector:_cmd]);
  super_impl(self, _cmd, size);

  if (size.width != old_size.width || size.height != old_size.height)
    [self shell]->OnSizeChanged();
}

// The contentView gets moved around during certain full-screen operations.
// This is less than ideal, and should eventually be removed.
void ViewDidMoveToSuperview(NSView* self, SEL _cmd) {
  if (!IsFramelessWindow(self) || ![self nuPrivate]->is_content_view) {
    auto super_impl = reinterpret_cast<decltype(&ViewDidMoveToSuperview)>(
        [[self superclass] instanceMethodForSelector:_cmd]);
    super_impl(self, _cmd);
    return;
  }

  [self setFrame:[[self superview] bounds]];
}

}  // namespace

bool IsNUView(id view) {
  return [view respondsToSelector:@selector(nuPrivate)];
}

bool NUViewMethodsInstalled(Class cl) {
  return class_getClassMethod(cl, @selector(nuInjected)) != nullptr;
}

void InstallNUViewMethods(Class cl) {
  class_addMethod(cl, @selector(nuInjected), (IMP)NUInjected, "B@:");
  class_addMethod(cl, @selector(shell), (IMP)GetShell, "^v@:");
  class_addMethod(cl, @selector(acceptsFirstResponder),
                  (IMP)AcceptsFirstResponder, "B@:");
  class_addMethod(cl, @selector(mouseDownCanMoveWindow),
                  (IMP)MouseDownCanMoveWindow, "B@:");
  class_addMethod(cl, @selector(enableTracking), (IMP)EnableTracking, "v@:");
  class_addMethod(cl, @selector(disableTracking), (IMP)DisableTracking, "v@:");
  class_addMethod(cl, @selector(setFrameSize:),
                  (IMP)SetFrameSize, "v@:{_NSSize=ff}");
  class_addMethod(cl, @selector(viewDidMoveToSuperview),
                  (IMP)ViewDidMoveToSuperview, "v@:");
}

}  // namespace nu
