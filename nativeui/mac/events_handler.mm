// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/events_handler.h"

#include <objc/objc-runtime.h>

#include "base/notreached.h"
#include "nativeui/events/event.h"
#include "nativeui/mac/dragging_info_mac.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_responder.h"
#include "nativeui/mac/nu_view.h"
#include "nativeui/mac/nu_window.h"
#include "nativeui/view.h"
#include "nativeui/window.h"

namespace nu {

namespace {

Responder* CheckAndGetNative(NSResponder* self) {
  CHECK([self respondsToSelector:@selector(shell)])
      << "Handler called for a responder other than NUView/NUWindow";
  Responder* shell = [self shell];
  DCHECK(shell) << "Handler called after responder is destroyed";
  return shell;
}

bool NUDummy(NSView* self, SEL _cmd) {
  return true;
}

void OnMouseEvent(NSResponder* self, SEL _cmd, NSEvent* event) {
  Responder* responder = CheckAndGetNative(self);
  if (!responder)
    return;

  // Emit the event to View.
  if (!DispatchMouseEvent(responder, event)) {
    // Transfer the event to super class.
    auto super_impl = reinterpret_cast<void (*)(NSResponder*, SEL, NSEvent*)>(
        [[self superclass] instanceMethodForSelector:_cmd]);
    super_impl(self, _cmd, event);
  }
}

void OnKeyEvent(NSResponder* self, SEL _cmd, NSEvent* event) {
  Responder* responder = CheckAndGetNative(self);
  if (!responder)
    return;

  // Emit the event to View.
  bool prevent_default = false;
  KeyEvent key_event(event, self);
  if (key_event.type == EventType::KeyDown)
    prevent_default = responder->on_key_down.Emit(responder, key_event);
  else if (key_event.type == EventType::KeyUp)
    prevent_default = responder->on_key_up.Emit(responder, key_event);
  else
    NOTREACHED();

  // Transfer the event to super class.
  if (!prevent_default) {
    auto super_impl = reinterpret_cast<void (*)(NSResponder*, SEL, NSEvent*)>(
        [[self superclass] instanceMethodForSelector:_cmd]);
    super_impl(self, _cmd, event);
  }
}

NSDragOperation DraggingEntered(NSView* self, SEL _cmd, id<NSDraggingInfo> info) {
  View* view = static_cast<View*>(CheckAndGetNative(self));
  if (!view || !view->handle_drag_enter)
    return NSDragOperationNone;

  DraggingInfoMac dragging_info(info);
  PointF point([self convertPoint:[info draggingLocation] fromView:nil]);
  int r = view->handle_drag_enter(view, &dragging_info, point);
  return [self nuPrivate]->last_drop_operation = r;
}

NSDragOperation DraggingUpdated(NSView* self, SEL _cmd, id<NSDraggingInfo> info) {
  View* view = static_cast<View*>(CheckAndGetNative(self));
  if (!view || !view->handle_drag_update)
    return [self nuPrivate]->last_drop_operation;

  DraggingInfoMac dragging_info(info);
  PointF point([self convertPoint:[info draggingLocation] fromView:nil]);
  int r = view->handle_drag_update(view, &dragging_info, point);
  return [self nuPrivate]->last_drop_operation = r;
}

void DraggingExited(NSView* self, SEL _cmd, id<NSDraggingInfo> info) {
  View* view = static_cast<View*>(CheckAndGetNative(self));
  if (!view || view->on_drag_leave.IsEmpty())
    return;

  DraggingInfoMac dragging_info(info);
  view->on_drag_leave.Emit(view, &dragging_info);
}

BOOL PerformDragOperation(NSView* self, SEL _cmd, id<NSDraggingInfo> info) {
  View* view = static_cast<View*>(CheckAndGetNative(self));
  if (!view)
    return NO;

  // Emit on_drag_leave to match the behavir on GTK.
  DraggingInfoMac dragging_info(info);
  view->on_drag_leave.Emit(view, &dragging_info);

  if (!view->handle_drop)
    return NO;
  PointF point([self convertPoint:[info draggingLocation] fromView:nil]);
  return view->handle_drop(view, &dragging_info, point);
}

NSDragOperation DraggingSessionSourceOperation(
    NSView* self, SEL _cmd, NSDraggingSession*, NSDraggingContext) {
  return [self nuPrivate]->supported_drag_operation;
}

void DraggingSessionEnded(NSView* self, SEL _cmd, NSDraggingSession*,
                          NSPoint, NSDragOperation operation) {
  [self nuPrivate]->drag_result = operation;
  static_cast<View*>([self shell])->CancelDrag();
}

}  // namespace

void AddMouseClickEventHandler(NSResponder* responder) {
  Class cl = [responder class];
  if ([cl instancesRespondToSelector:@selector(nuClickHandlerInjected)])
    return;
  class_addMethod(cl, @selector(nuClickHandlerInjected), (IMP)NUDummy, "B@:");

  class_addMethod(cl, @selector(mouseDown:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(rightMouseDown:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(otherMouseDown:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(mouseUp:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(rightMouseUp:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(otherMouseUp:), (IMP)OnMouseEvent, "v@:@");
}

void AddMouseMoveEventHandler(NSResponder* responder) {
  Class cl = [responder class];
  if ([cl instancesRespondToSelector:@selector(nuMoveHandlerInjected)])
    return;
  class_addMethod(cl, @selector(nuMoveHandlerInjected), (IMP)NUDummy, "B@:");

  class_addMethod(cl, @selector(mouseDragged:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(rightMouseDragged:), (IMP)OnMouseEvent,
                  "v@:@");
  class_addMethod(cl, @selector(otherMouseDragged:), (IMP)OnMouseEvent,
                  "v@:@");
  class_addMethod(cl, @selector(mouseEntered:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(mouseExited:), (IMP)OnMouseEvent, "v@:@");
}

void AddKeyEventHandler(NSResponder* responder) {
  Class cl = [responder class];
  if ([cl instancesRespondToSelector:@selector(nuKeyHandlerInjected)])
    return;
  class_addMethod(cl, @selector(nuKeyHandlerInjected), (IMP)NUDummy, "B@:");

  class_addMethod(cl, @selector(keyDown:), (IMP)OnKeyEvent, "v@:@");
  class_addMethod(cl, @selector(keyUp:), (IMP)OnKeyEvent, "v@:@");
  class_addMethod(cl, @selector(flagsChanged:), (IMP)OnKeyEvent, "v@:@");
}

void AddDragDropHandler(NSResponder* responder) {
  Class cl = [responder class];
  if ([cl instancesRespondToSelector:@selector(nuDropHandlerInjected)])
    return;
  class_addMethod(cl, @selector(nuDropHandlerInjected), (IMP)NUDummy, "B@:");

  class_addMethod(cl, @selector(draggingEntered:),
                  (IMP)DraggingEntered, "L@:@");
  class_addMethod(cl, @selector(draggingUpdated:),
                  (IMP)DraggingUpdated, "L@:@");
  class_addMethod(cl, @selector(draggingExited:),
                  (IMP)DraggingExited, "v@:@");
  class_addMethod(cl, @selector(performDragOperation:),
                  (IMP)PerformDragOperation, "B@:@");
  class_addMethod(cl, @selector(draggingSession:sourceOperationMaskForDraggingContext:),
                  (IMP)DraggingSessionSourceOperation, "L@:@L");
  class_addMethod(cl, @selector(draggingSession:endedAtPoint:operation:),
                  (IMP)DraggingSessionEnded, "v@:@{_NSPoint=ff}L");
}

bool DispatchMouseEvent(Responder* responder, NSEvent* event) {
  bool prevent_default = false;
  MouseEvent mouse_event(event, responder->GetNative());
  switch (mouse_event.type) {
    case EventType::MouseDown:
      prevent_default = responder->on_mouse_down.Emit(responder, mouse_event);
      break;
    case EventType::MouseUp:
      prevent_default = responder->on_mouse_up.Emit(responder, mouse_event);
      break;
    case EventType::MouseMove:
      responder->on_mouse_move.Emit(responder, mouse_event);
      prevent_default = true;
      break;
    case EventType::MouseEnter:
      [responder->GetNative() nuPrivate]->hovered = true;
      responder->on_mouse_enter.Emit(responder, mouse_event);
      prevent_default = true;
      break;
    case EventType::MouseLeave:
      [responder->GetNative() nuPrivate]->hovered = false;
      responder->on_mouse_leave.Emit(responder, mouse_event);
      prevent_default = true;
      break;
    default:
      break;
  }
  return prevent_default;
}

}  // namespace nu
