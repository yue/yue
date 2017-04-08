// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_SCREEN_WIN_H_
#define NATIVEUI_GFX_WIN_SCREEN_WIN_H_

#include <windows.h>

namespace nu {

// Returns |hwnd|'s scale factor.
float GetScaleFactorForHWND(HWND hwnd);

// Returns the equivalent scaling factor for |dpi|.
float GetScalingFactorFromDPI(int dpi);

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_SCREEN_WIN_H_
