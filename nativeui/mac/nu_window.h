// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_NU_WINDOW_H_
#define NATIVEUI_MAC_NU_WINDOW_H_

#include "base/mac/scoped_nsobject.h"
#include "nativeui/mac/nu_responder.h"

namespace nu {
class Window;
}

@interface NUWindow : NSWindow<NUResponder> {
 @private
  base::scoped_nsobject<NSTrackingArea> tracking_area_;
  nu::Window* shell_;
  bool can_resize_;
}
- (void)setShell:(nu::Window*)shell;
- (void)setWindowStyle:(NSUInteger)style on:(bool)yes;
- (bool)hasTrackingArea;
@end

#endif  // NATIVEUI_MAC_NU_WINDOW_H_
