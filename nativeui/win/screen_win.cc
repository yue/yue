// Copyright 2016 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE.chromium file.

#include "nativeui/win/screen_win.h"

#include <shellscalingapi.h>
#include <windows.h>

#include "base/hash/hash.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_hdc.h"
#include "base/win/win_util.h"
#include "nativeui/gfx/geometry/point_conversions.h"
#include "nativeui/gfx/geometry/rect.h"
#include "nativeui/screen.h"
#include "nativeui/win/util/win32_window.h"
#include "nativeui/win/window_win.h"

namespace nu {

namespace {

const float kDefaultDPI = 96.f;

Size GetDPI() {
  static int dpi_x = 0;
  static int dpi_y = 0;
  static bool should_initialize = true;

  if (should_initialize) {
    should_initialize = false;
    base::win::ScopedGetDC screen_dc(NULL);
    // This value is safe to cache for the life time of the app since the
    // user must logout to change the DPI setting. This value also applies
    // to all screens.
    dpi_x = GetDeviceCaps(screen_dc, LOGPIXELSX);
    dpi_y = GetDeviceCaps(screen_dc, LOGPIXELSY);
  }
  return Size(dpi_x, dpi_y);
}

// Gets the DPI for a particular monitor, or 0 if per-monitor DPI is nuot
// supported or can't be read.
int GetPerMonitorDPI(HMONITOR monitor) {
  // Most versions of Windows we will encounter are DPI-aware.
  if (!base::win::IsProcessPerMonitorDpiAware())
    return 0;

  static auto get_dpi_for_monitor_func = []() {
    using GetDpiForMonitorPtr = decltype(::GetDpiForMonitor)*;
    HMODULE shcore_dll = ::LoadLibrary(L"shcore.dll");
    if (shcore_dll) {
      return reinterpret_cast<GetDpiForMonitorPtr>(
          ::GetProcAddress(shcore_dll, "GetDpiForMonitor"));
    }
    return static_cast<GetDpiForMonitorPtr>(nullptr);
  }();

  if (!get_dpi_for_monitor_func)
    return 0;

  UINT dpi_x;
  UINT dpi_y;
  if (!SUCCEEDED(get_dpi_for_monitor_func(monitor, MDT_EFFECTIVE_DPI, &dpi_x,
                                          &dpi_y))) {
    return 0;
  }

  DCHECK_EQ(dpi_x, dpi_y);
  return static_cast<int>(dpi_x);
}

// Gets the raw monitor scale factor.
//
// Respects the forced device scale factor, and will fall back to the global
// scale factor if per-monitor DPI is not supported.
float GetMonitorScaleFactor(HMONITOR monitor) {
  DCHECK(monitor);
  int dpi = GetPerMonitorDPI(monitor);
  if (dpi)
    return GetScalingFactorFromDPI(dpi);
  return Screen::GetDefaultScaleFactor();
}

// Returns |point|, transformed from |from_origin|'s to |to_origin|'s
// coordinates, which differ by |scale_factor|.
PointF ScalePointRelative(const PointF& point,
                          const PointF& from_origin,
                          const PointF& to_origin,
                          const float scale_factor) {
  const PointF relative_point = point - from_origin.OffsetFromOrigin();
  const PointF scaled_relative_point =
      ScalePoint(relative_point, scale_factor);
  return scaled_relative_point + to_origin.OffsetFromOrigin();
}

}  // namespace

float GetScaleFactorForHWND(HWND hwnd) {
  HMONITOR monitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
  if (base::win::IsProcessPerMonitorDpiAware())
    return GetScalingFactorFromDPI(GetPerMonitorDPI(monitor));
  return Screen::GetDefaultScaleFactor();
}

int GetDPIFromScalingFactor(float device_scaling_factor) {
  return kDefaultDPI * device_scaling_factor;
}

float GetScalingFactorFromDPI(int dpi) {
  return static_cast<float>(dpi) / kDefaultDPI;
}

int GetSystemMetricsForScaleFactor(float scale_factor, int metric) {
  if (base::win::IsProcessPerMonitorDpiAware()) {
    static auto get_metric_for_dpi_func = []() {
      using GetSystemMetricsForDpiPtr = decltype(::GetSystemMetricsForDpi)*;
      HMODULE user32_dll = ::LoadLibrary(L"user32.dll");
      if (user32_dll) {
        return reinterpret_cast<GetSystemMetricsForDpiPtr>(
            ::GetProcAddress(user32_dll, "GetSystemMetricsForDpi"));
      }
      return static_cast<GetSystemMetricsForDpiPtr>(nullptr);
    }();

    if (get_metric_for_dpi_func) {
      return get_metric_for_dpi_func(metric,
                                     GetDPIFromScalingFactor(scale_factor));
    }
  }

  // Fallback for when we're running Windows 8.1, which doesn't support
  // GetSystemMetricsForDpi and yet does support per-process dpi awareness.
  int system_metrics_result = ::GetSystemMetrics(metric);
  return static_cast<int>(std::ceil(scale_factor * system_metrics_result /
                                    Screen::GetDefaultScaleFactor()));
}

int GetSystemMetricsForMonitor(HMONITOR monitor, int metric) {
  // We'll want to use GetSafeMonitorScaleFactor(), so if the monitor is not
  // specified pull up the primary display's HMONITOR.
  if (!monitor)
    monitor = ::MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY);

  float scale_factor = GetMonitorScaleFactor(monitor);

  // We'll then pull up the system metrics scaled by the appropriate amount.
  return GetSystemMetricsForScaleFactor(scale_factor, metric);
}

namespace internal {

class ScreenObserverImpl : public ScreenObserver,
                           public Win32Window {
 public:
  explicit ScreenObserverImpl(Screen* screen) : screen_(screen) {}

 private:
  // Win32Window:
  bool ProcessWindowMessage(HWND window,
                            UINT message,
                            WPARAM w_param,
                            LPARAM l_param,
                            LRESULT* result) override {
    if ((message == WM_DISPLAYCHANGE) ||
        (message == WM_SETTINGCHANGE && w_param == SPI_SETWORKAREA)) {
      screen_->NotifyDisplaysChange();
    }
    return false;
  }

  Screen* screen_;
};

// static
ScreenObserver* ScreenObserver::Create(Screen* screen) {
  return new ScreenObserverImpl(screen);
}

}  // namespace internal

// static
float Screen::GetDefaultScaleFactor() {
  return GetScalingFactorFromDPI(GetDPI().width());
}

Display Screen::GetDisplayNearestWindow(Window* window) {
  HMONITOR monitor = ::MonitorFromWindow(
      window->GetNative()->hwnd(), MONITOR_DEFAULTTONEAREST);
  if (!monitor)
    return GetPrimaryDisplay();
  return FindDisplay(monitor);
}

Display Screen::GetDisplayNearestPoint(const PointF& point) {
  Point screen_point = DIPToScreenPoint(point);
  HMONITOR monitor =
      ::MonitorFromPoint(screen_point.ToPOINT(), MONITOR_DEFAULTTONEAREST);
  if (!monitor)
    return GetPrimaryDisplay();
  return FindDisplay(monitor);
}

PointF Screen::GetCursorScreenPoint() {
  POINT pt;
  ::GetCursorPos(&pt);

  Point screen_point(pt);
  HMONITOR monitor = ::MonitorFromPoint(screen_point.ToPOINT(),
                                        MONITOR_DEFAULTTONEAREST);
  float scale_factor = monitor ? FindDisplay(monitor).scale_factor : 1.f;
  return ScalePoint(PointF(screen_point), 1.f / scale_factor);
}

Point Screen::DIPToScreenPoint(const PointF& point) {
  Display display;
  for (const Display& d : GetAllDisplays()) {
    if (d.bounds.Contains(point)) {
      display = d;
      break;
    } else if (d.bounds.origin().IsOrigin()) {
      display = d;
    }
  }
  return ToFlooredPoint(
      ScalePointRelative(point, display.bounds.origin(),
                         ScalePoint(display.bounds.origin(),
                                    display.scale_factor),
                         display.scale_factor));
}

// static
BOOL CALLBACK Screen::EnumMonitorCallback(HMONITOR monitor, HDC, LPRECT,
                                          LPARAM data) {
  auto* displays = reinterpret_cast<Screen::DisplayList*>(data);
  displays->emplace_back(Screen::CreateDisplayFromNative(monitor));
  return TRUE;
}

// static
uint32_t Screen::DisplayIdFromNative(HMONITOR monitor) {
  MONITORINFOEX monitor_info = {};
  monitor_info.cbSize = sizeof(monitor_info);
  ::GetMonitorInfoW(monitor, &monitor_info);
  return base::Hash(base::WideToUTF8(monitor_info.szDevice));
}

// static
NativeDisplay Screen::GetNativePrimaryDisplay() {
  return ::MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
}

// static
Display Screen::CreateDisplayFromNative(HMONITOR monitor) {
  MONITORINFOEX monitor_info = {};
  monitor_info.cbSize = sizeof(monitor_info);
  ::GetMonitorInfo(monitor, &monitor_info);

  Display display;
  display.id = base::Hash(base::WideToUTF8(monitor_info.szDevice));
  display.scale_factor = GetMonitorScaleFactor(monitor);
  display.bounds = ScaleRect(RectF(Rect(monitor_info.rcMonitor)),
                             1.f / display.scale_factor);
  display.work_area = ScaleRect(RectF(Rect(monitor_info.rcWork)),
                                1.f / display.scale_factor);
  return display;
}

// static
Screen::DisplayList Screen::CreateAllDisplays() {
  DisplayList displays;
  ::EnumDisplayMonitors(NULL, NULL,
                        &Screen::EnumMonitorCallback,
                        reinterpret_cast<LPARAM>(&displays));
  return displays;
}

}  // namespace nu
