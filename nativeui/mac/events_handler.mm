// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/events_handler.h"

#include <objc/objc-runtime.h>

#include "base/logging.h"
#include "base/mac/mac_util.h"
#include "nativeui/events/event.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/view_mac.h"
#include "nativeui/view.h"

namespace nu {

namespace {

bool NUInjected(NSView* self, SEL _cmd) {
  return true;
}

View* GetShell(NSView* self, SEL _cmd) {
  return [self nuPrivate]->shell;
}

void OnMouseEvent(NSView* self, SEL _cmd, NSEvent* event) {
  DCHECK([self respondsToSelector:@selector(shell)])
      << "Handler called for view other than NUView";
  View* view = [self shell];
  DCHECK(view);

  // Emit the event to View.
  if (!DispatchMouseEvent(view, event)) {
    // Transfer the event to super class.
    auto super_impl = reinterpret_cast<void (*)(NSView*, SEL, NSEvent*)>(
        [[self superclass] instanceMethodForSelector:_cmd]);
    super_impl(self, _cmd, event);
  }
}

void OnKeyEvent(NSView* self, SEL _cmd, NSEvent* event) {
  DCHECK([self respondsToSelector:@selector(shell)])
      << "Handler called for view other than NUView";
  View* view = [self shell];
  DCHECK(view);

  // Emit the event to View.
  bool prevent_default = false;
  KeyEvent key_event(event, self);
  if (key_event.type == EventType::KeyDown)
    prevent_default = view->on_key_down.Emit(view, key_event);
  else if (key_event.type == EventType::KeyUp)
    prevent_default = view->on_key_up.Emit(view, key_event);
  else
    NOTREACHED();

  // Transfer the event to super class.
  if (!prevent_default) {
    auto super_impl = reinterpret_cast<void (*)(NSView*, SEL, NSEvent*)>(
        [[self superclass] instanceMethodForSelector:_cmd]);
    super_impl(self, _cmd, event);
  }
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

void UpdateTrackingAreas(NSView* self, SEL _cmd) {
  // [super updateTrackingAreas]
  auto super_impl = reinterpret_cast<void (*)(NSView*, SEL)>(
      [[self superclass] instanceMethodForSelector:_cmd]);
  super_impl(self, _cmd);

  [self disableTracking];
  [self enableTracking];
}

}  // namespace

bool IsNUView(id view) {
  return [view respondsToSelector:@selector(nuPrivate)];
}

bool EventHandlerInstalled(Class cl) {
  return class_getClassMethod(cl, @selector(nuInjected)) != nullptr;
}

void AddMouseEventHandlerToClass(Class cl) {
  class_addMethod(cl, @selector(mouseDown:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(rightMouseDown:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(otherMouseDown:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(mouseUp:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(rightMouseUp:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(otherMouseUp:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(mouseMoved:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(mouseDragged:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(rightMouseDragged:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(otherMouseDragged:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(mouseEntered:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(mouseExited:), (IMP)OnMouseEvent, "v@:@");
}

void AddKeyEventHandlerToClass(Class cl) {
  class_addMethod(cl, @selector(keyDown:), (IMP)OnKeyEvent, "v@:@");
  class_addMethod(cl, @selector(keyUp:), (IMP)OnKeyEvent, "v@:@");
  class_addMethod(cl, @selector(flagsChanged:), (IMP)OnKeyEvent, "v@:@");
}

void AddViewMethodsToClass(Class cl) {
  class_addMethod(cl, @selector(nuInjected), (IMP)NUInjected, "B@:");
  class_addMethod(cl, @selector(shell), (IMP)GetShell, "^v@:");
  class_addMethod(cl, @selector(acceptsFirstResponder),
                  (IMP)AcceptsFirstResponder, "B@:");
  class_addMethod(cl, @selector(mouseDownCanMoveWindow),
                  (IMP)MouseDownCanMoveWindow, "B@:");
  class_addMethod(cl, @selector(enableTracking), (IMP)EnableTracking, "v@:");
  class_addMethod(cl, @selector(disableTracking), (IMP)DisableTracking, "v@:");

  // NSTrackingInVisibleRect doesn't work correctly with Lion's window resizing,
  // http://crbug.com/176725 / http://openradar.appspot.com/radar?id=2773401 .
  // Work around it by reinstalling the tracking area after window resize.
  // This AppKit bug is fixed on Yosemite, so we only apply this workaround on
  // 10.9.
  if (base::mac::IsOS10_9())
    class_addMethod(cl, @selector(updateTrackingAreas),
                    (IMP)UpdateTrackingAreas, "v@:");
}

bool DispatchMouseEvent(View* view, NSEvent* event) {
  bool prevent_default = false;
  MouseEvent mouse_event(event, view->GetNative());
  switch (mouse_event.type) {
    case EventType::MouseDown:
      prevent_default = view->on_mouse_down.Emit(view, mouse_event);
      break;
    case EventType::MouseUp:
      prevent_default = view->on_mouse_up.Emit(view, mouse_event);
      break;
    case EventType::MouseMove:
      view->on_mouse_move.Emit(view, mouse_event);
      prevent_default = true;
      break;
    case EventType::MouseEnter:
      view->on_mouse_enter.Emit(view, mouse_event);
      prevent_default = true;
      break;
    case EventType::MouseLeave:
      view->on_mouse_leave.Emit(view, mouse_event);
      prevent_default = true;
      break;
    default:
      break;
  }
  return prevent_default;
}

}  // namespace nu
