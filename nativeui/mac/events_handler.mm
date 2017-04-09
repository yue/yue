// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/events_handler.h"

#include <objc/objc-runtime.h>

#include "base/logging.h"

namespace nu {

namespace {

void OnMouseEvent(NSView* self, SEL _cmd, NSEvent* event) {
  if ([self respondsToSelector:@selector(shell)]) {
    LOG(ERROR) << "OnMouseEvent";
  } else {
    auto super_impl = reinterpret_cast<void (*)(NSView*, SEL, NSEvent*)>(
        [[self superclass] instanceMethodForSelector:_cmd]);
    super_impl(self, _cmd, event);
  }
}

void OnKeyEvent(NSView* self, SEL _cmd, NSEvent* event) {
  if ([self respondsToSelector:@selector(shell)])
    LOG(ERROR) << "OnKeyEvent";
}

void OnInsertText(NSView* self, SEL _cmd, NSString* text) {
  if ([self respondsToSelector:@selector(shell)])
    LOG(ERROR) << "OnInsertText";
}

BOOL OnPerformKeyEquivalent(NSView* self, SEL _cmd, NSEvent* event) {
  if ([self respondsToSelector:@selector(shell)])
    LOG(ERROR) << "OnPerformKeyEquivalent";
  return NO;
}

}  // namespace

void AddMouseEventMethodsToView(Class cl) {
  class_addMethod(cl, @selector(mouseDown:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(rightMouseDown:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(otherMouseDown:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(mouseUp:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(rightMouseUp:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(otherMouseUp:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(mouseDragged:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(rightMouseDragged:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(otherMouseDragged:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(mouseMoved:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(scrollWheel:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(mouseEntered:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(mouseExited:), (IMP)OnMouseEvent, "v@:@");
  class_addMethod(cl, @selector(magnifyWithEvent:), (IMP)OnMouseEvent, "v@:@");
}

void AddKeyEventMethodsToView(Class cl) {
  class_addMethod(cl, @selector(keyDown:), (IMP)OnKeyEvent, "v@:@");
  class_addMethod(cl, @selector(keyUp:), (IMP)OnKeyEvent, "v@:@");
  class_addMethod(cl, @selector(insertText:), (IMP)OnInsertText, "v@:@");
  class_addMethod(cl, @selector(performKeyEquivalent:),
                  (IMP)OnPerformKeyEquivalent, "c@:@");
}

}  // namespace nu
