// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_EVENTS_EVENT_H_
#define NATIVEUI_EVENTS_EVENT_H_

#include "nativeui/events/keyboard_codes.h"
#include "nativeui/gfx/geometry/point_f.h"
#include "nativeui/types.h"

namespace nu {

// Supported event types.
enum class EventType {
  Unknown,  // should never be used
  MouseDown,
  MouseUp,
  MouseMove,
  MouseEnter,
  MouseLeave,
  KeyDown,
  KeyUp,
};

// Base event type.
struct NATIVEUI_EXPORT Event {
  // Current keyboard states.
  static bool IsShiftPressed();
  static bool IsControlPressed();
  static bool IsAltPressed();
  static bool IsMetaPressed();

  // Event type.
  EventType type;

  // Current keyboard modifiers.
  int modifiers;

  // Time when event was created, starts from when machine was booted.
  uint32_t timestamp;

  // The underlying native event.
  NativeEvent native_event;

 protected:
  // Base Event class should nenver be created by user.
  Event(NativeEvent event, NativeResponder responder);
};

// Mouse click events.
struct NATIVEUI_EXPORT MouseEvent : public Event {
  // Create from the native event.
  MouseEvent(NativeEvent event, NativeResponder responder);

  int button;
  PointF position_in_view;
  PointF position_in_window;
};

// Key events.
struct NATIVEUI_EXPORT KeyEvent: public Event {
  // Create from the native event.
  KeyEvent(NativeEvent event, NativeResponder responder);

  KeyboardCode key;
};

}  // namespace nu

#endif  // NATIVEUI_EVENTS_EVENT_H_
