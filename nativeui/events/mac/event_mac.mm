// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/events/event.h"

#import <Cocoa/Cocoa.h>

#include "base/logging.h"
#include "nativeui/events/keyboard_codes.h"

namespace nu {

namespace {

EventType EventTypeFromNS(NSEventType type) {
  switch (type) {
    case NSLeftMouseDown:
    case NSRightMouseDown:
    case NSOtherMouseDown:
      return EventType::MouseDown;
    case NSLeftMouseUp:
    case NSRightMouseUp:
    case NSOtherMouseUp:
      return EventType::MouseUp;
    default:
      NOTREACHED();
  }
  return EventType::Unkown;
}

int EventModifiersFromNS(NSEventModifierFlags flags) {
  // KeyboardModifier is mapped from NSEventModifierFlags, so we only need to
  // filter out the ones we don't support.
  return flags & (MASK_SHIFT | MASK_CONTROL | MASK_ALT | MASK_COMMAND);
}

PointF FlipWindowPos(NSPoint point, NSWindow* window) {
  DCHECK(window);
  NSRect rect = [window contentRectForFrameRect:[window frame]];
  return PointF(point.x, NSHeight(rect) - point.y);
}

PointF FlipScreenPos(NSPoint point, NSScreen* screen) {
  DCHECK(screen);
  return PointF(point.x, NSMaxY([screen frame]) - point.y);
}

}  // namespace

Event::Event(NativeEvent event)
    : type(EventTypeFromNS([event type])),
      position_in_screen(FlipScreenPos([NSEvent mouseLocation],
                                       [[event window] screen])),
      modifiers(EventModifiersFromNS([event modifierFlags])),
      native_event(event) {
}

MouseEvent::MouseEvent(NativeEvent event, NativeView view)
    : Event(event),
      button([event buttonNumber]),
      position_in_window(FlipWindowPos([event locationInWindow],
                                       [event window])) {
}

}  // namespace nu
