// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/screen_win.h"

#include <shellscalingapi.h>
#include <windows.h>

#include "base/win/scoped_hdc.h"
#include "base/win/win_util.h"
#include "nativeui/gfx/geometry/rect.h"
#include "nativeui/gfx/screen.h"

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
  return GetScaleFactor();
}

}  // namespace

float GetScaleFactor() {
  return GetScalingFactorFromDPI(GetDPI().width());
}

float GetScaleFactorForHWND(HWND hwnd) {
  HMONITOR monitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
  if (base::win::IsProcessPerMonitorDpiAware())
    return GetScalingFactorFromDPI(GetPerMonitorDPI(monitor));
  return GetScaleFactor();
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
                                    GetScaleFactor()));
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

}  // namespace nu
