// Copyright 2020 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE.chromium file.

#include "nativeui/screen.h"

#import <Cocoa/Cocoa.h>

#include <map>

#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/mac/coordinate_conversion.h"
#include "nativeui/window.h"

namespace nu {

namespace {

// Returns the minimum Manhattan distance from |point| to corners of |screen|
// frame.
CGFloat GetMinimumDistanceToCorner(const NSPoint& point, NSScreen* screen) {
  NSRect frame = [screen frame];
  CGFloat distance =
      fabs(point.x - NSMinX(frame)) + fabs(point.y - NSMinY(frame));
  distance = std::min(
      distance, fabs(point.x - NSMaxX(frame)) + fabs(point.y - NSMinY(frame)));
  distance = std::min(
      distance, fabs(point.x - NSMinX(frame)) + fabs(point.y - NSMaxY(frame)));
  distance = std::min(
      distance, fabs(point.x - NSMaxX(frame)) + fabs(point.y - NSMaxY(frame)));
  return distance;
}

}  // namespace

namespace internal {

class ScreenObserverImpl : public ScreenObserver {
 public:
  explicit ScreenObserverImpl(Screen* screen) : screen_(screen) {
    CGDisplayRegisterReconfigurationCallback(DisplayReconfigurationCallBack,
                                             screen_);
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    screen_params_change_observer_.reset([[center
        addObserverForName:NSApplicationDidChangeScreenParametersNotification
                    object:nil
                     queue:nil
                usingBlock:^(NSNotification* notification) {
                             screen_->NotifyDisplaysChange();
                           }] retain]);
  }

  ~ScreenObserverImpl() {
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center removeObserver:screen_params_change_observer_];
    CGDisplayRemoveReconfigurationCallback(DisplayReconfigurationCallBack,
                                           screen_);
  }

 private:
  static void DisplayReconfigurationCallBack(CGDirectDisplayID display,
                                             CGDisplayChangeSummaryFlags flags,
                                             void* userInfo) {
    static_cast<Screen*>(userInfo)->NotifyDisplaysChange();
  }

  Screen* screen_;
  base::scoped_nsobject<id> screen_params_change_observer_;
};

// static
ScreenObserver* ScreenObserver::Create(Screen* screen) {
  return new ScreenObserverImpl(screen);
}

}  // namespace internal

// static
float Screen::GetDefaultScaleFactor() {
  NSScreen* screen = GetNativePrimaryDisplay();
  return screen ? [screen backingScaleFactor] : 1.f;
}

Display Screen::GetDisplayNearestWindow(Window* window) {
  DCHECK(window);
  NSScreen* screen = [window->GetNative() screen];
  return screen ? FindDisplay(screen) : GetPrimaryDisplay();
}

Display Screen::GetDisplayNearestPoint(const PointF& point) {
  NSArray* screens = [NSScreen screens];
  if ([screens count] <= 1)
    return GetPrimaryDisplay();

  NSPoint ns_point = point.ToCGPoint();
  NSScreen* primary = [screens firstObject];
  ns_point.y = NSMaxY([primary frame]) - ns_point.y;
  for (NSScreen* screen in screens) {
    if (NSMouseInRect(ns_point, [screen frame], NO))
      return FindDisplay(screen);
  }

  NSScreen* nearest_screen = primary;
  CGFloat min_distance = CGFLOAT_MAX;
  for (NSScreen* screen in screens) {
    CGFloat distance = GetMinimumDistanceToCorner(ns_point, screen);
    if (distance < min_distance) {
      min_distance = distance;
      nearest_screen = screen;
    }
  }
  return FindDisplay(nearest_screen);
}

PointF Screen::GetCursorScreenPoint() {
  return ScreenPointFromNSPoint([NSEvent mouseLocation]);
}

// static
uint32_t Screen::DisplayIdFromNative(NSScreen* screen) {
  return [[[screen deviceDescription]
      objectForKey:@"NSScreenNumber"] unsignedIntValue];
}

// static
NativeDisplay Screen::GetNativePrimaryDisplay() {
  return [[NSScreen screens] firstObject];
}

// static
Display Screen::CreateDisplayFromNative(NSScreen* screen) {
  Display display;
  display.id = DisplayIdFromNative(screen);
  display.scale_factor = [screen backingScaleFactor];
  display.internal = CGDisplayIsBuiltin(display.id);

  NSRect frame = [screen frame];
  NSRect visible_frame = [screen visibleFrame];
  NSScreen* primary = GetNativePrimaryDisplay();
  if ([screen isEqual:primary]) {
    display.bounds = RectF(frame);
    display.work_area = RectF(visible_frame);
    display.work_area.set_y(frame.size.height - visible_frame.origin.y -
                            visible_frame.size.height);
  } else {
    display.bounds = ScreenRectFromNSRect(frame);
    display.work_area = ScreenRectFromNSRect(visible_frame);
  }
  return display;
}

// static
Screen::DisplayList Screen::CreateAllDisplays() {
  // Don't just return all online displays.  This would include displays
  // that mirror other displays, which are not desired in this list.  It's
  // tempting to use the count returned by CGGetActiveDisplayList, but active
  // displays exclude sleeping displays, and those are desired.

  // It would be ridiculous to have this many displays connected, but
  // CGDirectDisplayID is just an integer, so supporting up to this many
  // doesn't hurt.
  CGDirectDisplayID online_displays[1024];
  CGDisplayCount online_display_count = 0;
  if (CGGetOnlineDisplayList(std::size(online_displays), online_displays,
                             &online_display_count) != kCGErrorSuccess) {
    return {CreatePrimaryDisplay()};
  }

  std::map<int64_t, NSScreen*> screen_ids_to_screens;
  for (NSScreen* screen in [NSScreen screens])
    screen_ids_to_screens[DisplayIdFromNative(screen)] = screen;

  DisplayList displays;
  for (CGDisplayCount i = 0; i < online_display_count; ++i) {
    CGDirectDisplayID online_display = online_displays[i];
    if (CGDisplayMirrorsDisplay(online_display) == kCGNullDirectDisplay) {
      // If this display doesn't mirror any other, include it in the list.
      // The primary display in a mirrored set will be counted, but those that
      // mirror it will not be.
      auto foundScreen = screen_ids_to_screens.find(online_display);
      if (foundScreen != screen_ids_to_screens.end())
        displays.emplace_back(CreateDisplayFromNative(foundScreen->second));
    }
  }

  if (displays.empty())
    return {CreatePrimaryDisplay()};
  return displays;
}

}  // namespace nu
