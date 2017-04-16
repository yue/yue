// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_VIEW_MAC_H_
#define NATIVEUI_MAC_VIEW_MAC_H_

#import <Cocoa/Cocoa.h>

#include "nativeui/gfx/color.h"

namespace nu {
class View;
struct NUPrivate;
}

@protocol NUView
- (nu::NUPrivate*)nuPrivate;
- (void)setNUBackgroundColor:(nu::Color)color;
@end

@interface NSView (NUViewMethods) <NUView>
- (nu::View*)shell;
- (void)setAcceptsFirstResponder:(BOOL)yes;
- (void)enableTracking;
- (void)disableTracking;
@end

#endif  // NATIVEUI_MAC_VIEW_MAC_H_
