// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/events/event.h"

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "base/logging.h"
#include "nativeui/events/gtk/keyboard_code_conversion_gtk.h"

namespace nu {

namespace {

EventType EventTypeFromGdk(GdkEventType type) {
  switch (type) {
    case GDK_BUTTON_PRESS:
      return EventType::MouseDown;
    case GDK_BUTTON_RELEASE:
      return EventType::MouseUp;
    case GDK_MOTION_NOTIFY:
      return EventType::MouseMove;
    case GDK_ENTER_NOTIFY:
      return EventType::MouseEnter;
    case GDK_LEAVE_NOTIFY:
      return EventType::MouseLeave;
    case GDK_KEY_PRESS:
      return EventType::KeyDown;
    case GDK_KEY_RELEASE:
      return EventType::KeyUp;
    default:
      return EventType::Unknown;
  }
}

int GetCurrentEventModifiers() {
  int modifiers = gdk_keymap_get_modifier_state(gdk_keymap_get_default());
  // KeyboardModifier is mapped from NSEventModifierFlags, so we only need to
  // filter out the ones we don't support.
  return modifiers & (MASK_SHIFT | MASK_CONTROL | MASK_ALT | MASK_META);
}

uint32_t TimestampFromGdkEvent(GdkEvent* event) {
  switch (event->any.type) {
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.time;
    case GDK_MOTION_NOTIFY:
      return event->motion.time;
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.time;
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
      return event->key.time;
    default:
      NOTREACHED();
      return 0;
  }
}

int ButtonFromGdkEvent(GdkEvent* event) {
  switch (event->any.type) {
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.button;
    default:
      return 0;
  }
}

PointF PositionFromGdkEvent(GdkEvent* event) {
  switch (event->any.type) {
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return PointF(event->button.x, event->button.y);
    case GDK_MOTION_NOTIFY:
      return PointF(event->motion.x, event->motion.y);
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return PointF(event->crossing.x, event->crossing.y);
    default:
      NOTREACHED();
      return PointF();
  }
}

}  // namespace

Event::Event(NativeEvent event, NativeView view)
    : type(EventTypeFromGdk(event->any.type)),
      modifiers(GetCurrentEventModifiers()),
      timestamp(TimestampFromGdkEvent(event)),
      native_event(event) {
}

MouseEvent::MouseEvent(NativeEvent event, NativeView view)
    : Event(event, view),
      button(ButtonFromGdkEvent(event)),
      position(PositionFromGdkEvent(event)) {
}

KeyEvent::KeyEvent(NativeEvent event, NativeView view)
    : Event(event, view),
      key(KeyboardCodeFromGdkKeyCode(event->key.keyval)) {
}

}  // namespace nu
