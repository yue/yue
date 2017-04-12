// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_EVENTS_EVENT_H_
#define NATIVEUI_EVENTS_EVENT_H_

#include "nativeui/gfx/geometry/point_f.h"
#include "nativeui/types.h"

namespace nu {

enum class EventType {
  Unkown,  // should never be used
  MouseDown,
  MouseUp,
};

struct NATIVEUI_EXPORT Event {
  EventType type = EventType::Unkown;
  PointF position_in_screen;
  int modifiers = 0;
  uint32_t timestamp = 0;

#if !defined(OS_WIN)
  NativeEvent native_event = nullptr;
#endif

  // Base Event class should not be created by user.
 protected:
#if defined(OS_WIN)
  explicit Event(EventType type);
#else
  explicit Event(NativeEvent event);
#endif
};

struct NATIVEUI_EXPORT MouseEvent : public Event {
  // Initialize from native event.
#if defined(OS_WIN)
  MouseEvent(EventType type, const PointF& position_in_window);
#else
  MouseEvent(NativeEvent event, NativeView view);
#endif

  int button = 0;
  PointF position_in_window;
  PointF position_in_view;
};

}  // namespace nu

#endif  // NATIVEUI_EVENTS_EVENT_H_
