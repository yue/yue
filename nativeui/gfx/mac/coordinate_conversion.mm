// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "nativeui/gfx/mac/coordinate_conversion.h"

#import <Cocoa/Cocoa.h>

#include "nativeui/gfx/geometry/point_f.h"
#include "nativeui/gfx/geometry/rect_f.h"

namespace nu {

namespace {

// The height of the primary display, which OSX defines as the monitor with the
// menubar. This is always at index 0.
CGFloat PrimaryDisplayHeight() {
  return NSMaxY([[[NSScreen screens] firstObject] frame]);
}

}  // namespace

NSRect ScreenRectToNSRect(const RectF& rect) {
  return NSMakeRect(rect.x(),
                    PrimaryDisplayHeight() - rect.y() - rect.height(),
                    rect.width(),
                    rect.height());
}

RectF ScreenRectFromNSRect(const NSRect& rect) {
  return RectF(rect.origin.x,
               PrimaryDisplayHeight() - rect.origin.y - rect.size.height,
               rect.size.width, rect.size.height);
}

NSPoint ScreenPointToNSPoint(const PointF& point) {
  return NSMakePoint(point.x(), PrimaryDisplayHeight() - point.y());
}

PointF ScreenPointFromNSPoint(const NSPoint& point) {
  return PointF(point.x, PrimaryDisplayHeight() - point.y);
}

}  // namespace nu
