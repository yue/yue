// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_EVENTS_HANDLER_H_
#define NATIVEUI_MAC_EVENTS_HANDLER_H_

#import <Cocoa/Cocoa.h>

namespace nu {

class Responder;

// Dynamically add event methods to NSView/NSWindow.
void AddMouseClickEventHandler(NSResponder* responder);
void AddMouseMoveEventHandler(NSResponder* responder);
void AddKeyEventHandler(NSResponder* responder);
void AddDragDropHandler(NSResponder* responder);

// Dispatch mouse events to a responder.
bool DispatchMouseEvent(Responder* responder, NSEvent* event);

}  // namespace nu

#endif  // NATIVEUI_MAC_EVENTS_HANDLER_H_
