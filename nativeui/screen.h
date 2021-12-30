// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_SCREEN_H_
#define NATIVEUI_SCREEN_H_

#include <memory>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "nativeui/gfx/geometry/rect_f.h"
#include "nativeui/signal.h"
#include "nativeui/types.h"

namespace nu {

class Screen;
class Window;

namespace internal {

// Internal: Interface for platform observers.
class ScreenObserver {
 public:
  static ScreenObserver* Create(Screen* screen);
  virtual ~ScreenObserver() {}
};

}  // namespace internal

struct NATIVEUI_EXPORT Display {
  uint32_t id = 0;
  float scale_factor = 1.f;
#if defined(OS_MAC)
  bool internal = false;
#endif
  RectF bounds;
  RectF work_area;

  bool operator==(uint32_t display_id) const;
  bool operator!=(const Display& other) const;
};

class NATIVEUI_EXPORT Screen : public SignalDelegate {
 public:
  using DisplayList = std::vector<Display>;

  ~Screen() override;

  static Screen* GetCurrent();

  // Internal: Get a default scale factor without initializing screen.
  static float GetDefaultScaleFactor();

  Display GetPrimaryDisplay();
  const DisplayList& GetAllDisplays();

  Display GetDisplayNearestWindow(Window* window);
  Display GetDisplayNearestPoint(const PointF& point);

  PointF GetCursorScreenPoint();

  // Internal: Used by observers to notify changes.
  void NotifyDisplaysChange();

  base::WeakPtr<Screen> GetWeakPtr() { return weak_factory_.GetWeakPtr(); }

  // Events.
  Signal<void(const Display&)> on_add_display;
  Signal<void(const Display&)> on_remove_display;
  Signal<void(const Display&)> on_update_display;

 private:
  friend class State;
  friend class internal::ScreenObserver;

  Screen();

  // SignalDelegate:
  void OnConnect(int) override;

  // Helpers used by platform implementations.
  Display FindDisplay(NativeDisplay native);
  static Display CreatePrimaryDisplay();

  // Platform implementations.
#if defined(OS_WIN)
  Point DIPToScreenPoint(const PointF& point);
  static BOOL CALLBACK EnumMonitorCallback(HMONITOR, HDC, LPRECT, LPARAM);
#endif
  static uint32_t DisplayIdFromNative(NativeDisplay native);
  static NativeDisplay GetNativePrimaryDisplay();
  static Display CreateDisplayFromNative(NativeDisplay native);
  static DisplayList CreateAllDisplays();

  std::unique_ptr<internal::ScreenObserver> observer_;
  DisplayList displays_;

  base::WeakPtrFactory<Screen> weak_factory_;
};

}  // namespace nu

#endif  // NATIVEUI_SCREEN_H_
