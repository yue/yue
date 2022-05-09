// Copyright 2017 Cheng Zhao. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/events/event.h"

#import <Cocoa/Cocoa.h>

#include "base/logging.h"
#include "nativeui/events/mac/keyboard_code_conversion_mac.h"

namespace nu {

namespace {

// Return true if the target modifier key is up. OS X has an "official" flag
// to test whether either left or right versions of a modifier key are held,
// and "unofficial" flags for the left and right versions independently. This
// function verifies that |target_key_mask| and |otherKeyMask| (which should be
// the left and right versions of a modifier) are consistent with with the
// state of |eitherKeyMask| (which should be the corresponding ""official"
// flag). If they are consistent, it tests |target_key_mask|; otherwise it tests
// |either_key_mask|.
inline bool IsModifierKeyUp(unsigned int flags,
                            unsigned int target_key_mask,
                            unsigned int other_key_mask,
                            unsigned int either_key_mask) {
  bool either_key_down = (flags & either_key_mask) != 0;
  bool target_key_down = (flags & target_key_mask) != 0;
  bool other_key_down = (flags & other_key_mask) != 0;
  if (either_key_down != (target_key_down || other_key_down))
    return !either_key_down;
  return !target_key_down;
}

bool IsKeyUpEvent(NSEvent* event) {
  // Unofficial bit-masks for left- and right-hand versions of modifier keys.
  // These values were determined empirically.
  const unsigned int kLeftControlKeyMask = 1 << 0;
  const unsigned int kLeftShiftKeyMask = 1 << 1;
  const unsigned int kRightShiftKeyMask = 1 << 2;
  const unsigned int kLeftCommandKeyMask = 1 << 3;
  const unsigned int kRightCommandKeyMask = 1 << 4;
  const unsigned int kLeftAlternateKeyMask = 1 << 5;
  const unsigned int kRightAlternateKeyMask = 1 << 6;
  const unsigned int kRightControlKeyMask = 1 << 13;

  switch ([event keyCode]) {
    case 54:  // Right Command
      return IsModifierKeyUp([event modifierFlags], kRightCommandKeyMask,
                             kLeftCommandKeyMask, NSCommandKeyMask);
    case 55:  // Left Command
      return IsModifierKeyUp([event modifierFlags], kLeftCommandKeyMask,
                             kRightCommandKeyMask, NSCommandKeyMask);

    case 57:  // Capslock
      return ([event modifierFlags] & NSAlphaShiftKeyMask) == 0;

    case 56:  // Left Shift
      return IsModifierKeyUp([event modifierFlags], kLeftShiftKeyMask,
                             kRightShiftKeyMask, NSShiftKeyMask);
    case 60:  // Right Shift
      return IsModifierKeyUp([event modifierFlags], kRightShiftKeyMask,
                             kLeftShiftKeyMask, NSShiftKeyMask);

    case 58:  // Left Alt
      return IsModifierKeyUp([event modifierFlags], kLeftAlternateKeyMask,
                             kRightAlternateKeyMask, NSAlternateKeyMask);
    case 61:  // Right Alt
      return IsModifierKeyUp([event modifierFlags], kRightAlternateKeyMask,
                             kLeftAlternateKeyMask, NSAlternateKeyMask);

    case 59:  // Left Ctrl
      return IsModifierKeyUp([event modifierFlags], kLeftControlKeyMask,
                             kRightControlKeyMask, NSControlKeyMask);
    case 62:  // Right Ctrl
      return IsModifierKeyUp([event modifierFlags], kRightControlKeyMask,
                             kLeftControlKeyMask, NSControlKeyMask);

    case 63:  // Function
      return ([event modifierFlags] & NSFunctionKeyMask) == 0;
  }
  return false;
}

EventType EventTypeFromNS(NSEvent* event) {
  switch ([event type]) {
    case NSLeftMouseDown:
    case NSRightMouseDown:
    case NSOtherMouseDown:
      return EventType::MouseDown;
    case NSLeftMouseUp:
    case NSRightMouseUp:
    case NSOtherMouseUp:
      return EventType::MouseUp;
    case NSLeftMouseDragged:
    case NSRightMouseDragged:
    case NSOtherMouseDragged:
    case NSMouseMoved:
      return EventType::MouseMove;
    case NSMouseEntered:
      return EventType::MouseEnter;
    case NSMouseExited:
      return EventType::MouseLeave;
    case NSKeyDown:
      return EventType::KeyDown;
    case NSKeyUp:
      return EventType::KeyUp;
    case NSFlagsChanged:
      return IsKeyUpEvent(event) ? EventType::KeyUp : EventType::KeyDown;
    default:
      return EventType::Unknown;
  }
}

int EventModifiersFromNS(NSEventModifierFlags flags) {
  // KeyboardModifier is mapped from NSEventModifierFlags, so we only need to
  // filter out the ones we don't support.
  return flags & (MASK_SHIFT | MASK_CONTROL | MASK_ALT | MASK_META);
}

PointF GetPosInWindow(NSEvent* event) {
  NSPoint point = [event locationInWindow];
  NSWindow* window = [event window];
  if (!window)
    return PointF(point);
  NSRect frame = [window contentRectForFrameRect:[window frame]];
  return PointF(point.x, NSHeight(frame) - point.y);
}

PointF GetPosInWindow(NSEvent* event, NSResponder* responder) {
  if ([responder isKindOfClass:[NSView class]] &&
      [static_cast<NSView*>(responder) isFlipped]) {
    return PointF([event locationInWindow]);
  }
  return GetPosInWindow(event);
}

PointF GetPosInView(NSEvent* event, NSResponder* responder) {
  if ([responder isKindOfClass:[NSView class]]) {
    NSView* view = static_cast<NSView*>(responder);
    NSPoint point = [view convertPoint:[event locationInWindow] fromView:nil];
    if ([view isFlipped])
      return PointF(point);
    NSRect frame = [view frame];
    return PointF(point.x, NSHeight(frame) - point.y);
  }
  return GetPosInWindow(event);
}

}  // namespace

// static
bool Event::IsShiftPressed() {
  return [NSEvent modifierFlags] & NSShiftKeyMask;
}

// static
bool Event::IsControlPressed() {
  return [NSEvent modifierFlags] & NSControlKeyMask;
}

// static
bool Event::IsAltPressed() {
  return [NSEvent modifierFlags] & NSAlternateKeyMask;
}

// static
bool Event::IsMetaPressed() {
  return [NSEvent modifierFlags] & NSCommandKeyMask;
}

Event::Event(NativeEvent event, NativeResponder responder)
    : type(EventTypeFromNS(event)),
      modifiers(EventModifiersFromNS([event modifierFlags])),
      timestamp([event timestamp] * 1000),
      native_event(event) {
}

MouseEvent::MouseEvent(NativeEvent event, NativeResponder responder)
    : Event(event, responder),
      button([event buttonNumber] + 1),
      position_in_view(GetPosInView(event, responder)),
      position_in_window(GetPosInWindow(event, responder)) {
}

KeyEvent::KeyEvent(NativeEvent event, NativeResponder responder)
    : Event(event, responder),
      key(KeyboardCodeFromNSEvent(event)) {
}

}  // namespace nu
