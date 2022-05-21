// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_NU_RESPONDER_H_
#define NATIVEUI_MAC_NU_RESPONDER_H_

#import <Cocoa/Cocoa.h>

namespace nu {
class Responder;
struct NUPrivate;
}

// Extended methods of NUResponder.
@interface NSResponder (NUResponderMethods)
- (nu::NUPrivate*)nuPrivate;
- (nu::Responder*)shell;
- (void)enableTracking;
- (void)disableTracking;
@end

namespace nu {

// Return whether a class is part of nativeui system.
bool IsNUResponder(id responder);

}  // namespace nu

#endif  // NATIVEUI_MAC_NU_RESPONDER_H_
