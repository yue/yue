// Copyright 2016 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE.chromium file.

#ifndef NATIVEUI_WIN_SCREEN_WIN_H_
#define NATIVEUI_WIN_SCREEN_WIN_H_

#include <windows.h>

namespace nu {

// Returns |hwnd|'s scale factor.
float GetScaleFactorForHWND(HWND hwnd);

// Returns the equivalent DPI for |device_scaling_factor|.
int GetDPIFromScalingFactor(float device_scaling_factor);

// Returns the equivalent scaling factor for |dpi|.
float GetScalingFactorFromDPI(int dpi);

// Returns the result of GetSystemMetrics for |metric| scaled to the specified
// |scale_factor|.
int GetSystemMetricsForScaleFactor(float scale_factor, int metric);

// Returns the result of GetSystemMetrics for |metric| scaled to |monitor|'s
// DPI. Use this function if you're already working with screen pixels, as
// this helps reduce any cascading rounding errors from DIP to the |monitor|'s
// DPI.
//
// Note that metrics which correspond to elements drawn by Windows
// (specifically frame and resize handles) will be scaled by DPI only and not
// by Text Zoom or other accessibility features.
int GetSystemMetricsForMonitor(HMONITOR monitor, int metric);

}  // namespace nu

#endif  // NATIVEUI_WIN_SCREEN_WIN_H_
