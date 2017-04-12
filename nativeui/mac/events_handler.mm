// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/events_handler.h"

#include <objc/objc-runtime.h>

#include "base/logging.h"
#include "nativeui/events/event.h"
#include "nativeui/mac/view_mac.h"

namespace nu {

namespace {

bool IsNUView(NSView* self, SEL _cmd) {
  return true;
}

void OnMouseEvent(NSView* self, SEL _cmd, NSEvent* event) {
  DCHECK([self respondsToSelector:@selector(shell)])
      << "Handler called for view other than NUView";
  View* view = [self shell];
  DCHECK(view);

  bool prevent_default = false;
  switch ([event type]) {
    case NSLeftMouseDown:
    case NSRightMouseDown:
    case NSOtherMouseDown:
      prevent_default = view->on_mouse_down.Emit(view, MouseEvent(event, self));
      break;
    case NSLeftMouseUp:
    case NSRightMouseUp:
    case NSOtherMouseUp:
      prevent_default = view->on_mouse_up.Emit(view, MouseEvent(event, self));
      break;
    default:
      NOTREACHED() << "Got unexpected event: " << [event type];
      break;
  }

  if (!prevent_default) {
    auto super_impl = reinterpret_cast<void (*)(NSView*, SEL, NSEvent*)>(
        [[self superclass] instanceMethodForSelector:_cmd]);
    super_impl(self, _cmd, event);
  }
}

void OnKeyEvent(NSView* self, SEL _cmd, NSEvent* event) {
}

}  // namespace

bool EventHandlerInstalled(Class cl) {
  return class_getClassMethod(cl, @selector(IsNUView)) != nullptr;
}

void AddMouseEventHandlerToClass(Class cl) {
  class_addMethod(cl, @selector(IsNUView), (IMP)IsNUView, "B@:");
  class_addMethod(cl, @selector(mouseDown:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(rightMouseDown:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(otherMouseDown:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(mouseUp:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(rightMouseUp:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(otherMouseUp:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(mouseMoved:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(mouseEntered:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(mouseExited:), (IMP)OnMouseEvent, "v@:@");
}

void AddKeyEventHandlerToClass(Class cl) {
  class_addMethod(cl, @selector(keyDown:), (IMP)OnKeyEvent, "v@:@");
  class_addMethod(cl, @selector(keyUp:), (IMP)OnKeyEvent, "v@:@");
}

}  // namespace nu
