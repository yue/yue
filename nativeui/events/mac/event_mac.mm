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
  return EventType::Unknown;
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

}  // namespace

Event::Event(NativeEvent event, NativeView view)
    : type(EventTypeFromNS([event type])),
      modifiers(EventModifiersFromNS([event modifierFlags])),
      timestamp([event timestamp] * 1000),
      native_event(event) {
}

MouseEvent::MouseEvent(NativeEvent event, NativeView view)
    : Event(event, view),
      button([event buttonNumber] + 1),
      position(FlipWindowPos([event locationInWindow], [event window])) {
}

}  // namespace nu
