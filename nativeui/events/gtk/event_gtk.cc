// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/events/event.h"

#include <gdk/gdk.h>
#include <gtk/gtk.h>

namespace nu {

namespace {

EventType EventTypeFromGdk(GdkEventType type) {
  switch (type) {
    case GDK_BUTTON_PRESS:
      return EventType::MouseDown;
    case GDK_BUTTON_RELEASE:
      return EventType::MouseUp;
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
  return flags & (MASK_SHIFT | MASK_CONTROL | MASK_ALT | MASK_META);
}

}  // namespace

Event::Event(NativeEvent event, NativeView view)
    : type(EventTypeFromGdk(event->any.type)),
      modifiers(GetCurrentEventModifiers()),
      timestamp(event->button.time),  // every input event has |time| property
      native_event(event) {
}

MouseEvent::MouseEvent(NativeEvent event, NativeView view)
    : Event(event, view),
      button(event->button.button),
      position(event->button.x, event->button.y) {
}

KeyEvent::KeyEvent(NativeEvent event, NativeView view)
    : KeyEvent(event, view),
      key(static_cast<KeyboardCode>(event->key.keyval)) {
}

}  // namespace nu
