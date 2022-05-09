// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_NU_VIEW_H_
#define NATIVEUI_MAC_NU_VIEW_H_

#include "nativeui/gfx/color.h"
#include "nativeui/mac/nu_responder.h"

namespace nu {
class Font;
struct NUPrivate;
}

// The methods that every View should implemented.
@protocol NUView
- (nu::NUPrivate*)nuPrivate;
- (void)setNUFont:(nu::Font*)font;
- (void)setNUColor:(nu::Color)color;
- (void)setNUBackgroundColor:(nu::Color)color;
- (void)setNUEnabled:(BOOL)enabled;
- (BOOL)isNUEnabled;
@end

// Extended methods of NUView.
@interface NSView (NUViewMethods) <NUResponder, NUView>
@end

namespace nu {

// Return whether a class is part of nativeui system.
bool IsNUView(id view);

// Add custom view methods to class.
void InstallNUViewMethods(Class cl);

}  // namespace nu

#endif  // NATIVEUI_MAC_NU_VIEW_H_
