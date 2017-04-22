// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/mouse_capture.h"

#include "nativeui/mac/events_handler.h"
#include "nativeui/view.h"

namespace nu {

MouseCapture::MouseCapture(View* view) : view_(view) {
  NSEventMask event_mask =
      NSLeftMouseDownMask | NSLeftMouseUpMask | NSRightMouseDownMask |
      NSRightMouseUpMask | NSMouseMovedMask | NSLeftMouseDraggedMask |
      NSRightMouseDraggedMask | NSMouseEnteredMask | NSMouseExitedMask |
      NSScrollWheelMask | NSOtherMouseDownMask | NSOtherMouseUpMask |
      NSOtherMouseDraggedMask;
  local_monitor_ = [NSEvent addLocalMonitorForEventsMatchingMask:event_mask
      handler:^NSEvent*(NSEvent* event) {
        ProcessCapturedMouseEvent(event);
        return nil;  // Swallow all local events.
      }];
  global_monitor_ = [NSEvent addGlobalMonitorForEventsMatchingMask:event_mask
      handler:^void(NSEvent* event) {
        ProcessCapturedMouseEvent(event);
      }];
}

MouseCapture::~MouseCapture() {
  [NSEvent removeMonitor:global_monitor_];
  [NSEvent removeMonitor:local_monitor_];
}

void MouseCapture::ProcessCapturedMouseEvent(NSEvent* event) {
  DispatchMouseEvent(view_, event);
}

}  // namespace nu
