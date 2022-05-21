// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_NU_WINDOW_H_
#define NATIVEUI_MAC_NU_WINDOW_H_

#import <Cocoa/Cocoa.h>

#include "base/mac/scoped_nsobject.h"

namespace nu {
class Window;
struct NUWindowPrivate;
}

// The methods that Window and Panel should implemented.
@protocol NUWindowMethods
- (nu::NUWindowPrivate*)nuPrivate;
@end

// Extended methods of NUWindow.
@interface NSWindow (NUWindowMethods) <NUWindowMethods>
- (nu::Window*)shell;
- (bool)hasTrackingArea;
- (void)updateTrackingAreas;
- (void)setWindowStyle:(NSUInteger)style on:(bool)yes;
@end

namespace nu {

// Add custom window methods to class.
void InstallNUWindowMethods(Class cl);

}  // namespace nu

#endif  // NATIVEUI_MAC_NU_WINDOW_H_
