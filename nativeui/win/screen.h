// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_SCREEN_H_
#define NATIVEUI_WIN_SCREEN_H_

#include <windows.h>

namespace nu {

// Returns |hwnd|'s scale factor.
float GetScaleFactorForHWND(HWND hwnd);

}  // namespace nu

#endif  // NATIVEUI_WIN_SCREEN_H_
