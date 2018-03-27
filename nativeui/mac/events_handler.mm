// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/events_handler.h"

#include <objc/objc-runtime.h>

#include "base/logging.h"
#include "nativeui/events/event.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"
#include "nativeui/view.h"

namespace nu {

namespace {

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

}  // namespace

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

bool DispatchMouseEvent(View* view, NSEvent* event) {
  bool prevent_default = false;
  NUPrivate* priv = [view->GetNative() nuPrivate];
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
      priv->hovered = true;
      view->on_mouse_enter.Emit(view, mouse_event);
      prevent_default = true;
      break;
    case EventType::MouseLeave:
      priv->hovered = false;
      view->on_mouse_leave.Emit(view, mouse_event);
      prevent_default = true;
      break;
    default:
      break;
  }
  return prevent_default;
}

}  // namespace nu
