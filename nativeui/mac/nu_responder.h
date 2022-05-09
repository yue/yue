// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_NU_RESPONDER_H_
#define NATIVEUI_MAC_NU_RESPONDER_H_

#import <Cocoa/Cocoa.h>

namespace nu {
class Responder;
}

// The methods that every View and Window should implemented.
@protocol NUResponder
- (nu::Responder*)shell;
- (void)enableTracking;
- (void)disableTracking;
@end

// Extended methods of NUResponder.
@interface NSResponder (NUResponderMethods) <NUResponder>
@end

#endif  // NATIVEUI_MAC_NU_RESPONDER_H_
