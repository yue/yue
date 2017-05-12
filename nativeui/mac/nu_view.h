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
struct NUPrivate;
}

// The methods that every View should implemented.
@protocol NUView
- (nu::NUPrivate*)nuPrivate;
- (void)setNUFont:(nu::Font*)font;
- (void)setNUColor:(nu::Color)color;
- (void)setNUBackgroundColor:(nu::Color)color;
@end

// Extended methods of NUView.
@interface NSView (NUViewMethods) <NUView>
- (nu::View*)shell;
- (void)enableTracking;
- (void)disableTracking;
@end

namespace nu {

// Return whether a class is part of nativeui system.
bool IsNUView(id view);

// Return whether a class has been installed with custom methods.
bool NUViewMethodsInstalled(Class cl);

// Add custom view methods to class.
void InstallNUViewMethods(Class cl);

}  // namespace nu

#endif  // NATIVEUI_MAC_NU_VIEW_H_
