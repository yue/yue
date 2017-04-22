// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/events/win/event_win.h"

#include "base/logging.h"
#include "nativeui/events/event.h"
#include "nativeui/gfx/geometry/point_conversions.h"
#include "nativeui/win/view_win.h"

namespace nu {

namespace {

EventType EventTypeFromMessage(Win32Message* msg) {
  switch (msg->message) {
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
      return EventType::MouseDown;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
      return EventType::MouseUp;
    case WM_MOUSEMOVE:
      if (msg->w_param == 0)
        return EventType::MouseMove;
      if (msg->w_param == 1)
        return EventType::MouseEnter;
      if (msg->w_param == 2)
        return EventType::MouseLeave;
      NOTREACHED();
      return EventType::Unknown;
    case WM_MOUSELEAVE:
      return EventType::MouseLeave;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
      return EventType::KeyDown;
    case WM_KEYUP:
    case WM_SYSKEYUP:
      return EventType::KeyUp;
    default:
      return EventType::Unknown;
  }
}

int GetCurrentModifiers() {
  int modifiers = 0;
  if ((::GetKeyState(VK_SHIFT) & 0x8000) == 0x8000)
    modifiers |= MASK_SHIFT;
  if ((::GetKeyState(VK_CONTROL) & 0x8000) == 0x8000)
    modifiers |= MASK_CONTROL;
  if ((::GetKeyState(VK_MENU) & 0x8000) == 0x8000)
    modifiers |= MASK_ALT;
  if ((::GetKeyState(VK_LWIN) & 0x8000) == 0x8000 ||
      (::GetKeyState(VK_RWIN) & 0x8000) == 0x8000)
    modifiers |= MASK_META;
  return modifiers;
}

int GetButtonNumber(UINT message) {
  switch (message) {
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
      return 1;
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
      return 2;
    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
      return 3;
    default:
      return 0;
  }
}

PointF GetPosInView(const Point& point, NativeView view) {
  Point view_point = point - view->size_allocation().OffsetFromOrigin();
  return ScalePoint(PointF(view_point), 1.f / view->scale_factor());
}

}  // namespace

Event::Event(NativeEvent event, NativeView view)
    : type(EventTypeFromMessage(event)),
      modifiers(GetCurrentModifiers()),
      timestamp(::GetTickCount()),
      native_event(event) {
}

MouseEvent::MouseEvent(NativeEvent event, NativeView view)
    : Event(event, view),
      button(GetButtonNumber(event->message)),
      position_in_view(GetPosInView(Point(event->l_param), view)),
      position_in_window(ScalePoint(PointF(Point(event->l_param)),
                                    1.f / view->scale_factor())) {
}

KeyEvent::KeyEvent(NativeEvent event, NativeView view)
    : Event(event, view),
      key(static_cast<KeyboardCode>(event->w_param)) {
}

}  // namespace nu
