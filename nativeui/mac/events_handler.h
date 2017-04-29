// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_EVENTS_HANDLER_H_
#define NATIVEUI_MAC_EVENTS_HANDLER_H_

#import <Cocoa/Cocoa.h>

namespace nu {

class View;

// Dynamically add event methods to a NSView.
void AddMouseEventHandlerToClass(Class cl);
void AddKeyEventHandlerToClass(Class cl);

// Dispatch mouse events to a view.
bool DispatchMouseEvent(View* view, NSEvent* event);

}  // namespace nu

#endif  // NATIVEUI_MAC_EVENTS_HANDLER_H_
