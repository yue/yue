// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_EVENTS_HANDLER_H_
#define NATIVEUI_MAC_EVENTS_HANDLER_H_

#import <Cocoa/Cocoa.h>

namespace nu {

// Dynamically add event methods to a NSView.
void AddMouseEventMethodsToView(Class cl);
void AddKeyEventMethodsToView(Class cl);

}  // namespace nu

#endif  // NATIVEUI_MAC_EVENTS_HANDLER_H_
