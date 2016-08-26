// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVEUI_GFX_MAC_COORDINATE_CONVERSION_H_
#define NATIVEUI_GFX_MAC_COORDINATE_CONVERSION_H_

#import <Foundation/Foundation.h>

#include "nativeui/nativeui_export.h"

namespace nu {

class Point;
class Rect;

// Convert a gfx::Rect specified with the origin at the top left of the primary
// display into AppKit secreen coordinates (origin at the bottom left).
NATIVEUI_EXPORT NSRect ScreenRectToNSRect(const Rect& rect);

// Convert an AppKit NSRect with origin in the bottom left of the primary
// display into a gfx::Rect with origin at the top left of the primary display.
NATIVEUI_EXPORT Rect ScreenRectFromNSRect(const NSRect& point);

// Convert a gfx::Point specified with the origin at the top left of the primary
// display into AppKit screen coordinates (origin at the bottom left).
NATIVEUI_EXPORT NSPoint ScreenPointToNSPoint(const Point& point);

// Convert an AppKit NSPoint with origin in the bottom left of the primary
// display into a gfx::Point with origin at the top left of the primary display.
NATIVEUI_EXPORT Point ScreenPointFromNSPoint(const NSPoint& point);

}  // namespace nu

#endif  // NATIVEUI_GFX_MAC_COORDINATE_CONVERSION_H_
