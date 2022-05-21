// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_NU_VIEW_H_
#define NATIVEUI_MAC_NU_VIEW_H_

#import <Cocoa/Cocoa.h>

#include "nativeui/gfx/color.h"

namespace nu {
class Font;
class View;
struct NUViewPrivate;
}

// The methods that every View should implemented.
@protocol NUViewMethods
- (nu::NUViewPrivate*)nuPrivate;
- (void)setNUFont:(nu::Font*)font;
- (void)setNUColor:(nu::Color)color;
- (void)setNUBackgroundColor:(nu::Color)color;
- (void)setNUEnabled:(BOOL)enabled;
- (BOOL)isNUEnabled;
@end

// Extended methods of NUView.
@interface NSView (NUViewMethods) <NUViewMethods>
- (nu::View*)shell;
@end

namespace nu {

// Add custom view methods to class.
void InstallNUViewMethods(Class cl);

}  // namespace nu

#endif  // NATIVEUI_MAC_NU_VIEW_H_
