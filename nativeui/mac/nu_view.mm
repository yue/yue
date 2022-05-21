// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/nu_view.h"

#include <objc/objc-runtime.h>

#include "base/mac/mac_util.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_responder.h"
#include "nativeui/mac/nu_window.h"
#include "nativeui/window.h"

namespace nu {

namespace {

// Returns whether the view belongs to a frameless window.
bool IsFramelessWindow(NSView* view) {
  if (![[view window] respondsToSelector:@selector(shell)])
    return false;
  Window* window = [[view window] shell];
  return !window->HasFrame();
}

// Create a fake mouse event.
NSEvent* FakeEvent(NSView* view, NSEventType type) {
  int windowNumber = [[view window] windowNumber];
  NSTimeInterval eventTime = [[NSApp currentEvent] timestamp];
  return [NSEvent enterExitEventWithType:type
                                location:NSZeroPoint
                           modifierFlags:0
                               timestamp:eventTime
                            windowNumber:windowNumber
                                 context:nil
                             eventNumber:0
                          trackingNumber:0xBADFACE
                                userData:nil];
}

// Following methods are overrided in NUView.

bool NUViewInjected(NSView* self, SEL _cmd) {
  return true;
}

Responder* GetViewShell(NSView* self, SEL _cmd) {
  return [self nuPrivate]->shell;
}

void EnableViewTracking(NSView* self, SEL _cmd) {
  NUViewPrivate* priv = [self nuPrivate];
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
  [self addTrackingArea:priv->tracking_area.get()];
}

void DisableViewTracking(NSView* self, SEL _cmd) {
  NUViewPrivate* priv = [self nuPrivate];
  if (priv->tracking_area) {
    [self removeTrackingArea:priv->tracking_area.get()];
    priv->tracking_area.reset();
  }
}

BOOL AcceptsFirstResponder(NSView* self, SEL _cmd) {
  return [self nuPrivate]->focusable;
}

BOOL MouseDownCanMoveWindow(NSView* self, SEL _cmd) {
  return [self nuPrivate]->draggable;
}

void ResetCursorRects(NSView* self, SEL _cmd) {
  NUViewPrivate* priv = [self nuPrivate];
  if (priv->cursor) {
    [self addCursorRect:[self bounds] cursor:priv->cursor];
  } else {
    auto super_impl = reinterpret_cast<decltype(&ResetCursorRects)>(
        [[self superclass] instanceMethodForSelector:_cmd]);
    super_impl(self, _cmd);
  }
}

// Fix mouseExited isn't called when mouse leaves trackingArea while scrolling:
// https://stackoverflow.com/questions/8979639
void UpdateViewTrackingAreas(NSView* self, SEL _cmd) {
  auto super_impl = reinterpret_cast<decltype(&UpdateViewTrackingAreas)>(
      [[self superclass] instanceMethodForSelector:_cmd]);
  super_impl(self, _cmd);

  NUViewPrivate* priv = [self nuPrivate];
  if (![self window] || !priv->tracking_area)
    return;

  [self disableTracking];

  NSPoint mouse = [[self window] mouseLocationOutsideOfEventStream];
  mouse = [self convertPoint:mouse fromView:nil];

  if (NSPointInRect(mouse, [self bounds])) {  // mouse in view.
    if (!priv->hovered)
      [self mouseEntered:FakeEvent(self, NSMouseEntered)];
  } else {  // mouse not in view.
    if (priv->hovered)
      [self mouseExited:FakeEvent(self, NSMouseExited)];
  }

  [self enableTracking];
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

void InstallNUViewMethods(Class cl) {
  if ([cl instancesRespondToSelector:@selector(nuInjected)])
    return;
  class_addMethod(cl, @selector(nuInjected), (IMP)NUViewInjected, "B@:");

  class_addMethod(cl, @selector(shell), (IMP)GetViewShell, "^v@:");
  class_addMethod(cl, @selector(enableTracking),
                  (IMP)EnableViewTracking, "v@:");
  class_addMethod(cl, @selector(disableTracking),
                  (IMP)DisableViewTracking, "v@:");

  class_addMethod(cl, @selector(acceptsFirstResponder),
                  (IMP)AcceptsFirstResponder, "B@:");
  class_addMethod(cl, @selector(mouseDownCanMoveWindow),
                  (IMP)MouseDownCanMoveWindow, "B@:");
  class_addMethod(cl, @selector(resetCursorRects),
                  (IMP)ResetCursorRects, "v@");
  class_addMethod(cl, @selector(updateTrackingAreas),
                  (IMP)UpdateViewTrackingAreas, "v@:");
  class_addMethod(cl, @selector(setFrameSize:),
                  (IMP)SetFrameSize, "v@:{_NSSize=ff}");
  class_addMethod(cl, @selector(viewDidMoveToSuperview),
                  (IMP)ViewDidMoveToSuperview, "v@:");
}

}  // namespace nu
